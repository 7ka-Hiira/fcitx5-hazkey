import Foundation

enum SocketError: Error {
  case pollFailed(Int32)
  case clientDisconnected(String)
  case readFailed(String, Int32)
  case incompleteRead(String)
  case messageTooLarge(UInt32)
  case writeFailed(String, Int32)
  case incompleteWrite(String)
}

func readData(from fd: Int32, count: Int) throws -> Data {
  var buffer = Data(count: count)
  var bytesRead = 0

  try buffer.withUnsafeMutableBytes { bufPtr in
    let baseAddress = bufPtr.baseAddress!.assumingMemoryBound(to: UInt8.self)

    while bytesRead < count {
      let n = read(fd, baseAddress.advanced(by: bytesRead), count - bytesRead)

      if n < 0 {
        if errno == EAGAIN || errno == EWOULDBLOCK {
          usleep(10_000)
          continue
        }
        throw SocketError.readFailed("Read failed", errno)
      }
      if n == 0 {
        throw SocketError.clientDisconnected("Client disconnected while reading")
      }
      bytesRead += n
    }
  }

  if bytesRead < count {
    throw SocketError.incompleteRead("Failed to read all bytes")
  }

  return buffer
}

func writeData(to fd: Int32, data: Data) throws {
  var bytesWritten = 0

  try data.withUnsafeBytes { bufPtr in
    let baseAddress = bufPtr.baseAddress!.assumingMemoryBound(to: UInt8.self)

    while bytesWritten < data.count {
      let n = write(fd, baseAddress.advanced(by: bytesWritten), data.count - bytesWritten)

      if n < 0 {
        if errno == EAGAIN || errno == EWOULDBLOCK {
          usleep(10_000)
          continue
        }
        throw SocketError.writeFailed("Write failed", errno)
      }
      if n == 0 {
        throw SocketError.clientDisconnected("Client disconnected while writing")
      }
      bytesWritten += n
    }
  }

  if bytesWritten < data.count {
    throw SocketError.incompleteWrite("Failed to write all bytes")
  }
}

@MainActor
func listenSocket() {
  var currentClientFd: Int32? = nil

  while true {
    var pollFds: [pollfd] = []

    // Always poll the server socket for new connections
    pollFds.append(pollfd(fd: fd, events: Int16(POLLIN), revents: 0))

    // If we have a current client, also poll it
    if let clientFd = currentClientFd {
      pollFds.append(pollfd(fd: clientFd, events: Int16(POLLIN), revents: 0))
    }

    let pollRes = poll(&pollFds, nfds_t(pollFds.count), 1000)  // 1 second timeout

    if pollRes < 0 {
      print("Poll failed: \(errno)")
      break
    }

    if pollRes == 0 {
      // Timeout - continue polling
      continue
    }

    // Check if server socket has a new connection
    if pollFds[0].revents & Int16(POLLIN) != 0 {
      var clientAddr = sockaddr()
      var clientLen: socklen_t = socklen_t(MemoryLayout<sockaddr>.size)
      let newClientFd = accept(fd, &clientAddr, &clientLen)

      if newClientFd != -1 {
        // If we already have a client, close it
        if let existingClientFd = currentClientFd {
          print("New client connecting, closing existing client: \(existingClientFd)")
          close(existingClientFd)
        }

        // Set up the new client
        print("Client connected: \(newClientFd)")

        // Make client non-blocking
        let clientFlags = fcntl(newClientFd, F_GETFL, 0)
        let fcntlRes = fcntl(newClientFd, F_SETFL, clientFlags | O_NONBLOCK)
        if fcntlRes != 0 {
          print("fcntl() failed for client")
          close(newClientFd)
          currentClientFd = nil
        } else {
          currentClientFd = newClientFd
        }
      }
    }

    // Check if current client has data
    if let clientFd = currentClientFd, pollFds.count > 1 {
      let clientEvents = Int32(pollFds[1].revents)

      if clientEvents & POLLHUP != 0 || clientEvents & POLLERR != 0 {
        print("Client disconnected or error: \(clientFd)")
        close(clientFd)
        currentClientFd = nil
        continue
      }

      if clientEvents & POLLIN != 0 {
        do {
          // Handle client request
          let maxMessageSize: UInt32 = 1024 * 1024  // 1MB limit

          // Read message length header (4 bytes, big-endian)
          print("Reading data from client \(clientFd)...")
          let lengthData = try readData(from: clientFd, count: 4)
          let readLen = lengthData.withUnsafeBytes { $0.load(as: UInt32.self).bigEndian }
          print("Message length: \(readLen)")

          // Sanity check
          guard readLen <= maxMessageSize else {
            throw SocketError.messageTooLarge(readLen)
          }

          // Read message body
          let query = try readData(from: clientFd, count: Int(readLen))
          print("Successfully read \(query.count) bytes")

          // Process and respond
          let response = processProto(data: query)
          print("Processed request, response size: \(response.count)")

          // Write response length
          var writeLen = UInt32(response.count).bigEndian
          let lengthHeader = withUnsafeBytes(of: &writeLen) { Data($0) }
          try writeData(to: clientFd, data: lengthHeader)

          // Write response body
          try writeData(to: clientFd, data: response)

          fsync(clientFd)
          print("Successfully wrote response")

        } catch let error as SocketError {
          switch error {
          case .clientDisconnected(let msg):
            print(msg)
          case .readFailed(let msg, let err):
            print("Read failed: \(msg), errno: \(err)")
          case .incompleteRead(let msg), .incompleteWrite(let msg):
            print(msg)
          case .messageTooLarge(let len):
            print("Message too large: \(len)")
          case .writeFailed(let msg, let err):
            print("Write failed: \(msg), errno: \(err)")
          default:
            print("Socket error: \(error)")
          }
          print("Closing client connection due to error: \(clientFd)")
          close(clientFd)
          currentClientFd = nil
        } catch {
          print("An unexpected error occurred: \(error)")
          print("Closing client connection: \(clientFd)")
          close(clientFd)
          currentClientFd = nil
        }
      }
    }
  }
}
