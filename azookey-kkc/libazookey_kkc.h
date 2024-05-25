#ifndef LIBAZOOKEY_KKC_H
#define LIBAZOOKEY_KKC_H

#ifdef __cplusplus
extern "C" {
#endif

struct KkcConfig;
typedef struct KkcConfig KkcConfig;
typedef struct ComposingText ComposingText;

KkcConfig *kkc_get_config(void);
void kkc_free_config(KkcConfig *kkcConfigPtr);

ComposingText *kkc_get_composing_text(void);
void kkc_free_composing_text(ComposingText *composingTextPtr);

void kkc_input_text(ComposingText *composingTextPtr, char *stringPtr);
void kkc_delete_backward(ComposingText *composingTextPtr);
void kkc_delete_forward(ComposingText *composingTextPtr);

int kkc_move_cursor(ComposingText *composingTextPtr, int cursorIndex);

char **kkc_get_candidates(ComposingText *composingTextPtr,
                          const KkcConfig *kkcConfigPtr, bool isPredictMode,
                          int nBest);
void kkc_free_candidates(char **candidatesPtr);

#ifdef __cplusplus
}
#endif

#endif  // LIBAZOOKEY_KKC_H