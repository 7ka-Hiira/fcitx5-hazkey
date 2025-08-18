#ifndef LLAMA_DLOPEN_TOOLS_H
#define LLAMA_DLOPEN_TOOLS_H

#include "llama.h"

#ifdef __cplusplus
extern "C" {
#endif

// Context parameter setters
void llama_dlopen_tools_set_n_ctx(struct llama_context_params* params,
                                  uint32_t n_ctx);
void llama_dlopen_tools_set_n_threads(struct llama_context_params* params,
                                      int32_t n_threads);
void llama_dlopen_tools_set_n_threads_batch(struct llama_context_params* params,
                                            int32_t n_threads_batch);
void llama_dlopen_tools_set_n_batch(struct llama_context_params* params,
                                    uint32_t n_batch);

// Model parameter setters
void llama_dlopen_tools_set_n_gpu_layers(struct llama_model_params* params,
                                         int32_t n_gpu_layers);
void llama_dlopen_tools_set_use_mmap(struct llama_model_params* params,
                                     bool use_mmap);

// Parameter struct allocators and initializers
struct llama_context_params* llama_dlopen_tools_alloc_context_params();
struct llama_model_params* llama_dlopen_tools_alloc_model_params();
void llama_dlopen_tools_free_context_params(
    struct llama_context_params* params);
void llama_dlopen_tools_free_model_params(struct llama_model_params* params);

// Wrapper functions that take pointers and call real functions with
// dereferenced structs
struct llama_model* llama_dlopen_tools_model_load_from_file_ptr(
    const char* path_model, struct llama_model_params* params);
struct llama_context* llama_dlopen_tools_init_from_model_ptr(
    struct llama_model* model, struct llama_context_params* params);

// Batch allocation helpers
struct llama_batch* llama_dlopen_tools_alloc_batch(int32_t n_tokens,
                                                   int32_t embd,
                                                   int32_t n_seq_max);
void llama_dlopen_tools_free_batch(struct llama_batch* batch);

// Batch field accessor functions
int32_t llama_dlopen_tools_batch_get_n_tokens(struct llama_batch* batch);
void llama_dlopen_tools_batch_set_n_tokens(struct llama_batch* batch,
                                           int32_t n_tokens);
llama_token* llama_dlopen_tools_batch_get_token(struct llama_batch* batch);
void llama_dlopen_tools_batch_set_token(struct llama_batch* batch,
                                        llama_token* token);
llama_pos* llama_dlopen_tools_batch_get_pos(struct llama_batch* batch);
void llama_dlopen_tools_batch_set_pos(struct llama_batch* batch,
                                      llama_pos* pos);
int32_t* llama_dlopen_tools_batch_get_n_seq_id(struct llama_batch* batch);
void llama_dlopen_tools_batch_set_n_seq_id(struct llama_batch* batch,
                                           int32_t* n_seq_id);
llama_seq_id** llama_dlopen_tools_batch_get_seq_id(struct llama_batch* batch);
void llama_dlopen_tools_batch_set_seq_id(struct llama_batch* batch,
                                         llama_seq_id** seq_id);
int8_t* llama_dlopen_tools_batch_get_logits(struct llama_batch* batch);
void llama_dlopen_tools_batch_set_logits(struct llama_batch* batch,
                                         int8_t* logits);

// Decode wrapper function that dereferences batch pointer
int32_t llama_dlopen_tools_decode(struct llama_context* ctx,
                                  struct llama_batch* batch);

#ifdef __cplusplus
}
#endif

#endif  // LLAMA_DLOPEN_TOOLS_H
