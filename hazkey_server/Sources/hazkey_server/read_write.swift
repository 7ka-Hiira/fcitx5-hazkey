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
