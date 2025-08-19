#include "llama.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// Helper macro to panic for functions that should return a value
#define PANIC()                                                             \
    do {                                                                    \
        fprintf(stderr, "PANIC: llama stub function called: %s at %s:%d\n", \
                __func__, __FILE__, __LINE__);                              \
        abort();                                                            \
    } while (0)

// Model parameters functions
LLAMA_API struct llama_model_params llama_model_default_params(void) {
    PANIC();
}

LLAMA_API struct llama_context_params llama_context_default_params(void) {
    PANIC();
}

LLAMA_API struct llama_sampler_chain_params llama_sampler_chain_default_params(
    void) {
    PANIC();
}

LLAMA_API struct llama_model_quantize_params
llama_model_quantize_default_params(void) {
    PANIC();
}

// Backend functions
LLAMA_API void llama_backend_init(void) {
    // Do nothing
}

LLAMA_API void llama_backend_free(void) {
    // Do nothing
}

LLAMA_API void llama_numa_init(enum ggml_numa_strategy numa) {
    (void)numa;
    // Do nothing
}

LLAMA_API void llama_attach_threadpool(struct llama_context* ctx,
                                       ggml_threadpool_t threadpool,
                                       ggml_threadpool_t threadpool_batch) {
    (void)ctx;
    (void)threadpool;
    (void)threadpool_batch;
    // Do nothing
}

LLAMA_API void llama_detach_threadpool(struct llama_context* ctx) {
    (void)ctx;
    // Do nothing
}

// Model loading functions
LLAMA_API struct llama_model* llama_model_load_from_file(
    const char* path_model, struct llama_model_params params) {
    (void)path_model;
    (void)params;
    PANIC();
}

LLAMA_API struct llama_model* llama_model_load_from_splits(
    const char** paths, size_t n_paths, struct llama_model_params params) {
    (void)paths;
    (void)n_paths;
    (void)params;
    PANIC();
}

LLAMA_API void llama_model_free(struct llama_model* model) {
    (void)model;
    // Do nothing
}

// Context functions
LLAMA_API struct llama_context* llama_init_from_model(
    struct llama_model* model, struct llama_context_params params) {
    (void)model;
    (void)params;
    PANIC();
}

LLAMA_API void llama_free(struct llama_context* ctx) {
    (void)ctx;
    // Do nothing
}

// Utility functions
LLAMA_API int64_t llama_time_us(void) { PANIC(); }

LLAMA_API size_t llama_max_devices(void) { PANIC(); }

LLAMA_API bool llama_supports_mmap(void) { PANIC(); }

LLAMA_API bool llama_supports_mlock(void) { PANIC(); }

LLAMA_API bool llama_supports_gpu_offload(void) { PANIC(); }

LLAMA_API bool llama_supports_rpc(void) { PANIC(); }

// Context info functions
LLAMA_API uint32_t llama_n_ctx(const struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

LLAMA_API uint32_t llama_n_batch(const struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

LLAMA_API uint32_t llama_n_ubatch(const struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

LLAMA_API uint32_t llama_n_seq_max(const struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

// Model info functions
LLAMA_API const struct llama_model* llama_get_model(
    const struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

LLAMA_API enum llama_pooling_type llama_pooling_type(
    const struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

LLAMA_API const struct llama_vocab* llama_model_get_vocab(
    const struct llama_model* model) {
    (void)model;
    PANIC();
}

LLAMA_API enum llama_rope_type llama_model_rope_type(
    const struct llama_model* model) {
    (void)model;
    PANIC();
}

LLAMA_API int32_t llama_model_n_ctx_train(const struct llama_model* model) {
    (void)model;
    PANIC();
}

LLAMA_API int32_t llama_model_n_embd(const struct llama_model* model) {
    (void)model;
    PANIC();
}

LLAMA_API int32_t llama_model_n_layer(const struct llama_model* model) {
    (void)model;
    PANIC();
}

LLAMA_API int32_t llama_model_n_head(const struct llama_model* model) {
    (void)model;
    PANIC();
}

LLAMA_API float llama_model_rope_freq_scale_train(
    const struct llama_model* model) {
    (void)model;
    PANIC();
}

// Vocabulary functions
LLAMA_API enum llama_vocab_type llama_vocab_type(
    const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API int32_t llama_vocab_n_tokens(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API int32_t llama_model_meta_val_str(const struct llama_model* model,
                                           const char* key, char* buf,
                                           size_t buf_size) {
    (void)model;
    (void)key;
    (void)buf;
    (void)buf_size;
    PANIC();
}

LLAMA_API int32_t llama_model_meta_count(const struct llama_model* model) {
    (void)model;
    PANIC();
}

LLAMA_API int32_t llama_model_meta_key_by_index(const struct llama_model* model,
                                                int32_t i, char* buf,
                                                size_t buf_size) {
    (void)model;
    (void)i;
    (void)buf;
    (void)buf_size;
    PANIC();
}

LLAMA_API int32_t llama_model_meta_val_str_by_index(
    const struct llama_model* model, int32_t i, char* buf, size_t buf_size) {
    (void)model;
    (void)i;
    (void)buf;
    (void)buf_size;
    PANIC();
}

LLAMA_API int32_t llama_model_desc(const struct llama_model* model, char* buf,
                                   size_t buf_size) {
    (void)model;
    (void)buf;
    (void)buf_size;
    PANIC();
}

LLAMA_API uint64_t llama_model_size(const struct llama_model* model) {
    (void)model;
    PANIC();
}

LLAMA_API const char* llama_model_chat_template(const struct llama_model* model,
                                                const char* tmpl_name) {
    (void)model;
    (void)tmpl_name;
    PANIC();
}

LLAMA_API uint64_t llama_model_n_params(const struct llama_model* model) {
    (void)model;
    PANIC();
}

LLAMA_API bool llama_model_has_encoder(const struct llama_model* model) {
    (void)model;
    PANIC();
}

LLAMA_API bool llama_model_has_decoder(const struct llama_model* model) {
    (void)model;
    PANIC();
}

LLAMA_API llama_token
llama_model_decoder_start_token(const struct llama_model* model) {
    (void)model;
    PANIC();
}

LLAMA_API bool llama_model_is_recurrent(const struct llama_model* model) {
    (void)model;
    PANIC();
}

LLAMA_API uint32_t
llama_model_quantize(const char* fname_inp, const char* fname_out,
                     const llama_model_quantize_params* params) {
    (void)fname_inp;
    (void)fname_out;
    (void)params;
    PANIC();
}

// LoRA functions
LLAMA_API struct llama_adapter_lora* llama_adapter_lora_init(
    struct llama_model* model, const char* path_lora) {
    (void)model;
    (void)path_lora;
    PANIC();
}

LLAMA_API void llama_adapter_lora_free(struct llama_adapter_lora* adapter) {
    (void)adapter;
    // Do nothing
}

LLAMA_API int32_t llama_set_adapter_lora(struct llama_context* ctx,
                                         struct llama_adapter_lora* adapter,
                                         float scale) {
    (void)ctx;
    (void)adapter;
    (void)scale;
    PANIC();
}

LLAMA_API int32_t llama_rm_adapter_lora(struct llama_context* ctx,
                                        struct llama_adapter_lora* adapter) {
    (void)ctx;
    (void)adapter;
    PANIC();
}

LLAMA_API void llama_clear_adapter_lora(struct llama_context* ctx) {
    (void)ctx;
    // Do nothing
}

LLAMA_API int32_t llama_apply_adapter_cvec(struct llama_context* ctx,
                                           const float* data, size_t len,
                                           int32_t n_embd, int32_t il_start,
                                           int32_t il_end) {
    (void)ctx;
    (void)data;
    (void)len;
    (void)n_embd;
    (void)il_start;
    (void)il_end;
    PANIC();
}

// KV cache functions
LLAMA_API struct llama_kv_cache_view llama_kv_cache_view_init(
    const struct llama_context* ctx, int32_t n_seq_max) {
    (void)ctx;
    (void)n_seq_max;
    PANIC();
}

LLAMA_API void llama_kv_cache_view_free(struct llama_kv_cache_view* view) {
    (void)view;
    // Do nothing
}

LLAMA_API void llama_kv_cache_view_update(const struct llama_context* ctx,
                                          struct llama_kv_cache_view* view) {
    (void)ctx;
    (void)view;
    // Do nothing
}

LLAMA_API int32_t
llama_get_kv_cache_token_count(const struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

LLAMA_API int32_t
llama_get_kv_cache_used_cells(const struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

LLAMA_API void llama_kv_cache_clear(struct llama_context* ctx) {
    (void)ctx;
    // Do nothing
}

LLAMA_API bool llama_kv_cache_seq_rm(struct llama_context* ctx,
                                     llama_seq_id seq_id, llama_pos p0,
                                     llama_pos p1) {
    (void)ctx;
    (void)seq_id;
    (void)p0;
    (void)p1;
    PANIC();
}

LLAMA_API void llama_kv_cache_seq_cp(struct llama_context* ctx,
                                     llama_seq_id seq_id_src,
                                     llama_seq_id seq_id_dst, llama_pos p0,
                                     llama_pos p1) {
    (void)ctx;
    (void)seq_id_src;
    (void)seq_id_dst;
    (void)p0;
    (void)p1;
    // Do nothing
}

LLAMA_API void llama_kv_cache_seq_keep(struct llama_context* ctx,
                                       llama_seq_id seq_id) {
    (void)ctx;
    (void)seq_id;
    // Do nothing
}

LLAMA_API void llama_kv_cache_seq_add(struct llama_context* ctx,
                                      llama_seq_id seq_id, llama_pos p0,
                                      llama_pos p1, llama_pos delta) {
    (void)ctx;
    (void)seq_id;
    (void)p0;
    (void)p1;
    (void)delta;
    // Do nothing
}

LLAMA_API void llama_kv_cache_seq_div(struct llama_context* ctx,
                                      llama_seq_id seq_id, llama_pos p0,
                                      llama_pos p1, int32_t d) {
    (void)ctx;
    (void)seq_id;
    (void)p0;
    (void)p1;
    (void)d;
    // Do nothing
}

LLAMA_API llama_pos llama_kv_cache_seq_pos_max(struct llama_context* ctx,
                                               llama_seq_id seq_id) {
    (void)ctx;
    (void)seq_id;
    PANIC();
}

LLAMA_API void llama_kv_cache_defrag(struct llama_context* ctx) {
    (void)ctx;
    // Do nothing
}

LLAMA_API void llama_kv_cache_update(struct llama_context* ctx) {
    (void)ctx;
    // Do nothing
}

LLAMA_API bool llama_kv_cache_can_shift(struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

// State management functions
LLAMA_API size_t llama_state_get_size(struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

LLAMA_API size_t llama_state_get_data(struct llama_context* ctx, uint8_t* dst,
                                      size_t size) {
    (void)ctx;
    (void)dst;
    (void)size;
    PANIC();
}

LLAMA_API size_t llama_state_set_data(struct llama_context* ctx,
                                      const uint8_t* src, size_t size) {
    (void)ctx;
    (void)src;
    (void)size;
    PANIC();
}

LLAMA_API bool llama_state_load_file(struct llama_context* ctx,
                                     const char* path_session,
                                     llama_token* tokens_out,
                                     size_t n_token_capacity,
                                     size_t* n_token_count_out) {
    (void)ctx;
    (void)path_session;
    (void)tokens_out;
    (void)n_token_capacity;
    (void)n_token_count_out;
    PANIC();
}

LLAMA_API bool llama_state_save_file(struct llama_context* ctx,
                                     const char* path_session,
                                     const llama_token* tokens,
                                     size_t n_token_count) {
    (void)ctx;
    (void)path_session;
    (void)tokens;
    (void)n_token_count;
    PANIC();
}

LLAMA_API size_t llama_state_seq_get_size(struct llama_context* ctx,
                                          llama_seq_id seq_id) {
    (void)ctx;
    (void)seq_id;
    PANIC();
}

LLAMA_API size_t llama_state_seq_get_data(struct llama_context* ctx,
                                          uint8_t* dst, size_t size,
                                          llama_seq_id seq_id) {
    (void)ctx;
    (void)dst;
    (void)size;
    (void)seq_id;
    PANIC();
}

LLAMA_API size_t llama_state_seq_set_data(struct llama_context* ctx,
                                          const uint8_t* src, size_t size,
                                          llama_seq_id dest_seq_id) {
    (void)ctx;
    (void)src;
    (void)size;
    (void)dest_seq_id;
    PANIC();
}

LLAMA_API size_t llama_state_seq_save_file(struct llama_context* ctx,
                                           const char* filepath,
                                           llama_seq_id seq_id,
                                           const llama_token* tokens,
                                           size_t n_token_count) {
    (void)ctx;
    (void)filepath;
    (void)seq_id;
    (void)tokens;
    (void)n_token_count;
    PANIC();
}

LLAMA_API size_t llama_state_seq_load_file(struct llama_context* ctx,
                                           const char* filepath,
                                           llama_seq_id dest_seq_id,
                                           llama_token* tokens_out,
                                           size_t n_token_capacity,
                                           size_t* n_token_count_out) {
    (void)ctx;
    (void)filepath;
    (void)dest_seq_id;
    (void)tokens_out;
    (void)n_token_capacity;
    (void)n_token_count_out;
    PANIC();
}

// Batch functions
LLAMA_API struct llama_batch llama_batch_get_one(llama_token* tokens,
                                                 int32_t n_tokens) {
    (void)tokens;
    (void)n_tokens;
    PANIC();
}

LLAMA_API struct llama_batch llama_batch_init(int32_t n_tokens_alloc,
                                              int32_t embd, int32_t n_seq_max) {
    (void)n_tokens_alloc;
    (void)embd;
    (void)n_seq_max;
    PANIC();
}

LLAMA_API void llama_batch_free(struct llama_batch batch) {
    (void)batch;
    // Do nothing
}

// Inference functions
LLAMA_API int32_t llama_encode(struct llama_context* ctx,
                               struct llama_batch batch) {
    (void)ctx;
    (void)batch;
    PANIC();
}

LLAMA_API int32_t llama_decode(struct llama_context* ctx,
                               struct llama_batch batch) {
    (void)ctx;
    (void)batch;
    PANIC();
}

// Threading functions
LLAMA_API void llama_set_n_threads(struct llama_context* ctx, int32_t n_threads,
                                   int32_t n_threads_batch) {
    (void)ctx;
    (void)n_threads;
    (void)n_threads_batch;
    // Do nothing
}

LLAMA_API int32_t llama_n_threads(struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

LLAMA_API int32_t llama_n_threads_batch(struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

LLAMA_API void llama_set_embeddings(struct llama_context* ctx,
                                    bool embeddings) {
    (void)ctx;
    (void)embeddings;
    // Do nothing
}

LLAMA_API void llama_set_causal_attn(struct llama_context* ctx,
                                     bool causal_attn) {
    (void)ctx;
    (void)causal_attn;
    // Do nothing
}

LLAMA_API void llama_set_abort_callback(struct llama_context* ctx,
                                        ggml_abort_callback abort_callback,
                                        void* abort_callback_data) {
    (void)ctx;
    (void)abort_callback;
    (void)abort_callback_data;
    // Do nothing
}

LLAMA_API void llama_synchronize(struct llama_context* ctx) {
    (void)ctx;
    // Do nothing
}

// Output functions
LLAMA_API float* llama_get_logits(struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

LLAMA_API float* llama_get_logits_ith(struct llama_context* ctx, int32_t i) {
    (void)ctx;
    (void)i;
    PANIC();
}

LLAMA_API float* llama_get_embeddings(struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

LLAMA_API float* llama_get_embeddings_ith(struct llama_context* ctx,
                                          int32_t i) {
    (void)ctx;
    (void)i;
    PANIC();
}

LLAMA_API float* llama_get_embeddings_seq(struct llama_context* ctx,
                                          llama_seq_id seq_id) {
    (void)ctx;
    (void)seq_id;
    PANIC();
}

// Vocabulary access functions
LLAMA_API const char* llama_vocab_get_text(const struct llama_vocab* vocab,
                                           llama_token token) {
    (void)vocab;
    (void)token;
    PANIC();
}

LLAMA_API float llama_vocab_get_score(const struct llama_vocab* vocab,
                                      llama_token token) {
    (void)vocab;
    (void)token;
    PANIC();
}

LLAMA_API enum llama_token_attr llama_vocab_get_attr(
    const struct llama_vocab* vocab, llama_token token) {
    (void)vocab;
    (void)token;
    PANIC();
}

LLAMA_API bool llama_vocab_is_eog(const struct llama_vocab* vocab,
                                  llama_token token) {
    (void)vocab;
    (void)token;
    PANIC();
}

LLAMA_API bool llama_vocab_is_control(const struct llama_vocab* vocab,
                                      llama_token token) {
    (void)vocab;
    (void)token;
    PANIC();
}

// Special token functions
LLAMA_API llama_token llama_vocab_bos(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API llama_token llama_vocab_eos(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API llama_token llama_vocab_eot(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API llama_token llama_vocab_sep(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API llama_token llama_vocab_nl(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API llama_token llama_vocab_pad(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API bool llama_vocab_get_add_bos(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API bool llama_vocab_get_add_eos(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API llama_token llama_vocab_fim_pre(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API llama_token llama_vocab_fim_suf(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API llama_token llama_vocab_fim_mid(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API llama_token llama_vocab_fim_pad(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API llama_token llama_vocab_fim_rep(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API llama_token llama_vocab_fim_sep(const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

// Tokenization functions
LLAMA_API int32_t llama_tokenize(const struct llama_vocab* vocab,
                                 const char* text, int32_t text_len,
                                 llama_token* tokens, int32_t n_tokens_max,
                                 bool add_special, bool parse_special) {
    (void)vocab;
    (void)text;
    (void)text_len;
    (void)tokens;
    (void)n_tokens_max;
    (void)add_special;
    (void)parse_special;
    PANIC();
}

LLAMA_API int32_t llama_token_to_piece(const struct llama_vocab* vocab,
                                       llama_token token, char* buf,
                                       int32_t length, int32_t lstrip,
                                       bool special) {
    (void)vocab;
    (void)token;
    (void)buf;
    (void)length;
    (void)lstrip;
    (void)special;
    PANIC();
}

LLAMA_API int32_t llama_detokenize(const struct llama_vocab* vocab,
                                   const llama_token* tokens, int32_t n_tokens,
                                   char* text, int32_t text_len_max,
                                   bool remove_special, bool unparse_special) {
    (void)vocab;
    (void)tokens;
    (void)n_tokens;
    (void)text;
    (void)text_len_max;
    (void)remove_special;
    (void)unparse_special;
    PANIC();
}

// Chat template functions
LLAMA_API int32_t llama_chat_apply_template(
    const char* tmpl, const struct llama_chat_message* chat, size_t n_msg,
    bool add_ass, char* buf, int32_t length) {
    (void)tmpl;
    (void)chat;
    (void)n_msg;
    (void)add_ass;
    (void)buf;
    (void)length;
    PANIC();
}

LLAMA_API int32_t llama_chat_builtin_templates(const char** output,
                                               size_t len) {
    (void)output;
    (void)len;
    PANIC();
}

// Sampler functions
LLAMA_API struct llama_sampler* llama_sampler_init(
    const struct llama_sampler_i* iface, llama_sampler_context_t ctx) {
    (void)iface;
    (void)ctx;
    PANIC();
}

LLAMA_API const char* llama_sampler_name(const struct llama_sampler* smpl) {
    (void)smpl;
    PANIC();
}

LLAMA_API void llama_sampler_accept(struct llama_sampler* smpl,
                                    llama_token token) {
    (void)smpl;
    (void)token;
    // Do nothing
}

LLAMA_API void llama_sampler_apply(struct llama_sampler* smpl,
                                   struct llama_token_data_array* cur_p) {
    (void)smpl;
    (void)cur_p;
    // Do nothing
}

LLAMA_API void llama_sampler_reset(struct llama_sampler* smpl) {
    (void)smpl;
    // Do nothing
}

LLAMA_API struct llama_sampler* llama_sampler_clone(
    const struct llama_sampler* smpl) {
    (void)smpl;
    PANIC();
}

LLAMA_API void llama_sampler_free(struct llama_sampler* smpl) {
    (void)smpl;
    // Do nothing
}

// Sampler chain functions
LLAMA_API struct llama_sampler* llama_sampler_chain_init(
    struct llama_sampler_chain_params params) {
    (void)params;
    PANIC();
}

LLAMA_API void llama_sampler_chain_add(struct llama_sampler* chain,
                                       struct llama_sampler* smpl) {
    (void)chain;
    (void)smpl;
    // Do nothing
}

LLAMA_API struct llama_sampler* llama_sampler_chain_get(
    const struct llama_sampler* chain, int32_t i) {
    (void)chain;
    (void)i;
    PANIC();
}

LLAMA_API int32_t llama_sampler_chain_n(const struct llama_sampler* chain) {
    (void)chain;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_chain_remove(
    struct llama_sampler* chain, int32_t i) {
    (void)chain;
    (void)i;
    PANIC();
}

// Built-in samplers
LLAMA_API struct llama_sampler* llama_sampler_init_greedy(void) { PANIC(); }

LLAMA_API struct llama_sampler* llama_sampler_init_dist(uint32_t seed) {
    (void)seed;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_top_k(int32_t k) {
    (void)k;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_top_p(float p,
                                                         size_t min_keep) {
    (void)p;
    (void)min_keep;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_min_p(float p,
                                                         size_t min_keep) {
    (void)p;
    (void)min_keep;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_typical(float p,
                                                           size_t min_keep) {
    (void)p;
    (void)min_keep;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_temp(float temp) {
    (void)temp;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_temp_ext(float temp,
                                                            float delta,
                                                            float exponent) {
    (void)temp;
    (void)delta;
    (void)exponent;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_xtc(float probability,
                                                       float threshold,
                                                       size_t min_keep,
                                                       uint32_t seed) {
    (void)probability;
    (void)threshold;
    (void)min_keep;
    (void)seed;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_top_n_sigma(
    float threshold) {
    (void)threshold;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_mirostat(
    int32_t n_vocab, uint32_t seed, float tau, float eta, int32_t m) {
    (void)n_vocab;
    (void)seed;
    (void)tau;
    (void)eta;
    (void)m;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_mirostat_v2(uint32_t seed,
                                                               float tau,
                                                               float eta) {
    (void)seed;
    (void)tau;
    (void)eta;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_grammar(
    const struct llama_vocab* vocab, const char* grammar_str,
    const char* grammar_root) {
    (void)vocab;
    (void)grammar_str;
    (void)grammar_root;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_grammar_lazy(
    const struct llama_vocab* vocab, const char* grammar_str,
    const char* grammar_root, const char** trigger_words,
    size_t num_trigger_words, const llama_token* trigger_tokens,
    size_t num_trigger_tokens) {
    (void)vocab;
    (void)grammar_str;
    (void)grammar_root;
    (void)trigger_words;
    (void)num_trigger_words;
    (void)trigger_tokens;
    (void)num_trigger_tokens;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_penalties(
    int32_t penalty_last_n, float penalty_repeat, float penalty_freq,
    float penalty_present) {
    (void)penalty_last_n;
    (void)penalty_repeat;
    (void)penalty_freq;
    (void)penalty_present;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_dry(
    const struct llama_vocab* vocab, int32_t n_ctx_train, float dry_multiplier,
    float dry_base, int32_t dry_allowed_length, int32_t dry_penalty_last_n,
    const char** seq_breakers, size_t num_breakers) {
    (void)vocab;
    (void)n_ctx_train;
    (void)dry_multiplier;
    (void)dry_base;
    (void)dry_allowed_length;
    (void)dry_penalty_last_n;
    (void)seq_breakers;
    (void)num_breakers;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_logit_bias(
    int32_t n_vocab, int32_t n_logit_bias, const llama_logit_bias* logit_bias) {
    (void)n_vocab;
    (void)n_logit_bias;
    (void)logit_bias;
    PANIC();
}

LLAMA_API struct llama_sampler* llama_sampler_init_infill(
    const struct llama_vocab* vocab) {
    (void)vocab;
    PANIC();
}

LLAMA_API uint32_t llama_sampler_get_seed(const struct llama_sampler* smpl) {
    (void)smpl;
    PANIC();
}

LLAMA_API llama_token llama_sampler_sample(struct llama_sampler* smpl,
                                           struct llama_context* ctx,
                                           int32_t idx) {
    (void)smpl;
    (void)ctx;
    (void)idx;
    PANIC();
}

// Path functions
LLAMA_API int llama_split_path(char* split_path, size_t maxlen,
                               const char* path_prefix, int split_no,
                               int split_count) {
    (void)split_path;
    (void)maxlen;
    (void)path_prefix;
    (void)split_no;
    (void)split_count;
    PANIC();
}

LLAMA_API int llama_split_prefix(char* split_prefix, size_t maxlen,
                                 const char* split_path, int split_no,
                                 int split_count) {
    (void)split_prefix;
    (void)maxlen;
    (void)split_path;
    (void)split_no;
    (void)split_count;
    PANIC();
}

// System info
LLAMA_API const char* llama_print_system_info(void) { PANIC(); }

// Logging
LLAMA_API void llama_log_set(ggml_log_callback log_callback, void* user_data) {
    (void)log_callback;
    (void)user_data;
    // Do nothing
}

// Performance functions
LLAMA_API struct llama_perf_context_data llama_perf_context(
    const struct llama_context* ctx) {
    (void)ctx;
    PANIC();
}

LLAMA_API void llama_perf_context_print(const struct llama_context* ctx) {
    (void)ctx;
    // Do nothing
}

LLAMA_API void llama_perf_context_reset(struct llama_context* ctx) {
    (void)ctx;
    // Do nothing
}

LLAMA_API struct llama_perf_sampler_data llama_perf_sampler(
    const struct llama_sampler* smpl) {
    (void)smpl;
    PANIC();
}

LLAMA_API void llama_perf_sampler_print(const struct llama_sampler* smpl) {
    (void)smpl;
    // Do nothing
}

LLAMA_API void llama_perf_sampler_reset(struct llama_sampler* smpl) {
    (void)smpl;
    // Do nothing
}
