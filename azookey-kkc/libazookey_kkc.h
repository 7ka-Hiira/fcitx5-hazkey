#ifndef LIBAZOOKEY_KKC_H
#define LIBAZOOKEY_KKC_H

#ifdef __cplusplus
extern "C" {
#endif

struct KkcConfig;
typedef struct KkcConfig KkcConfig;

KkcConfig* kkc_get_config(void);
void kkc_free_config(KkcConfig* kkcConfigPtr);

char* kkc_ascii2hiragana(const char* stringPtr, const KkcConfig* kkcConfigPtr);
void  kkc_free_ascii2hiragana(char* stringPtr);

char**  kkc_convert(const char* stringPtr, const KkcConfig* kkcConfigPtr);
void    kkc_free_convert(char** stringPtr);

#ifdef __cplusplus
}
#endif

#endif // LIBAZOOKEY_KKC_H