#include "llama-dlopen-tools.h"

#include <stdbool.h>
#include <stdlib.h>

// Context parameter setters
void llama_dlopen_tools_set_n_ctx(struct llama_context_params* params,
                                  uint32_t n_ctx) {
    if (params != NULL) {
        params->n_ctx = n_ctx;
    }
}

void llama_dlopen_tools_set_n_threads(struct llama_context_params* params,
                                      int32_t n_threads) {
    if (params != NULL) {
        params->n_threads = n_threads;
    }
}

void llama_dlopen_tools_set_n_threads_batch(struct llama_context_params* params,
                                            int32_t n_threads_batch) {
    if (params != NULL) {
        params->n_threads_batch = n_threads_batch;
    }
}

void llama_dlopen_tools_set_n_batch(struct llama_context_params* params,
                                    uint32_t n_batch) {
    if (params != NULL) {
        params->n_batch = n_batch;
    }
}

// Model parameter setters
void llama_dlopen_tools_set_n_gpu_layers(struct llama_model_params* params,
                                         int32_t n_gpu_layers) {
    if (params != NULL) {
        params->n_gpu_layers = n_gpu_layers;
    }
}

void llama_dlopen_tools_set_use_mmap(struct llama_model_params* params,
                                     bool use_mmap) {
    if (params != NULL) {
        params->use_mmap = use_mmap;
    }
}

// Parameter struct allocators and initializers
struct llama_context_params* llama_dlopen_tools_alloc_context_params() {
    struct llama_context_params* params =
        malloc(sizeof(struct llama_context_params));
    if (params != NULL) {
        *params = llama_context_default_params();
    }
    return params;
}

struct llama_model_params* llama_dlopen_tools_alloc_model_params() {
    struct llama_model_params* params =
        malloc(sizeof(struct llama_model_params));
    if (params != NULL) {
        *params = llama_model_default_params();
    }
    return params;
}

void llama_dlopen_tools_free_context_params(
    struct llama_context_params* params) {
    if (params != NULL) {
        free(params);
    }
}

void llama_dlopen_tools_free_model_params(struct llama_model_params* params) {
    if (params != NULL) {
        free(params);
    }
}

// Wrapper functions that take pointers and call real functions with
// dereferenced structs
struct llama_model* llama_dlopen_tools_model_load_from_file_ptr(
    const char* path_model, struct llama_model_params* params) {
    if (params == NULL) {
        return NULL;
    }
    return llama_model_load_from_file(path_model, *params);
}

struct llama_context* llama_dlopen_tools_init_from_model_ptr(
    struct llama_model* model, struct llama_context_params* params) {
    if (params == NULL || model == NULL) {
        return NULL;
    }
    return llama_init_from_model(model, *params);
}

// Batch allocation helpers
struct llama_batch* llama_dlopen_tools_alloc_batch(int32_t n_tokens,
                                                   int32_t embd,
                                                   int32_t n_seq_max) {
    struct llama_batch* batch = malloc(sizeof(struct llama_batch));
    if (batch != NULL) {
        *batch = llama_batch_init(n_tokens, embd, n_seq_max);
    }
    return batch;
}

void llama_dlopen_tools_free_batch(struct llama_batch* batch) {
    if (batch != NULL) {
        llama_batch_free(*batch);
        free(batch);
    }
}

// Batch field accessor functions
int32_t llama_dlopen_tools_batch_get_n_tokens(struct llama_batch* batch) {
    return batch ? batch->n_tokens : 0;
}

void llama_dlopen_tools_batch_set_n_tokens(struct llama_batch* batch,
                                           int32_t n_tokens) {
    if (batch) {
        batch->n_tokens = n_tokens;
    }
}

llama_token* llama_dlopen_tools_batch_get_token(struct llama_batch* batch) {
    return batch ? batch->token : NULL;
}

void llama_dlopen_tools_batch_set_token(struct llama_batch* batch,
                                        llama_token* token) {
    if (batch) {
        batch->token = token;
    }
}

llama_pos* llama_dlopen_tools_batch_get_pos(struct llama_batch* batch) {
    return batch ? batch->pos : NULL;
}

void llama_dlopen_tools_batch_set_pos(struct llama_batch* batch,
                                      llama_pos* pos) {
    if (batch) {
        batch->pos = pos;
    }
}

int32_t* llama_dlopen_tools_batch_get_n_seq_id(struct llama_batch* batch) {
    return batch ? batch->n_seq_id : NULL;
}

void llama_dlopen_tools_batch_set_n_seq_id(struct llama_batch* batch,
                                           int32_t* n_seq_id) {
    if (batch) {
        batch->n_seq_id = n_seq_id;
    }
}

llama_seq_id** llama_dlopen_tools_batch_get_seq_id(struct llama_batch* batch) {
    return batch ? batch->seq_id : NULL;
}

void llama_dlopen_tools_batch_set_seq_id(struct llama_batch* batch,
                                         llama_seq_id** seq_id) {
    if (batch) {
        batch->seq_id = seq_id;
    }
}

int8_t* llama_dlopen_tools_batch_get_logits(struct llama_batch* batch) {
    return batch ? batch->logits : NULL;
}

void llama_dlopen_tools_batch_set_logits(struct llama_batch* batch,
                                         int8_t* logits) {
    if (batch) {
        batch->logits = logits;
    }
}

// Decode wrapper function that dereferences batch pointer
int32_t llama_dlopen_tools_decode(struct llama_context* ctx,
                                  struct llama_batch* batch) {
    if (batch == NULL || ctx == NULL) {
        return -1;
    }
    return llama_decode(ctx, *batch);
}
