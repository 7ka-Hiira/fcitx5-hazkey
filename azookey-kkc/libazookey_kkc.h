#ifndef LIBAZOOKEY_KKC_H
#define LIBAZOOKEY_KKC_H

#ifdef __cplusplus
extern "C" {
#endif

struct ComposingText;
typedef struct KkcConfig KkcConfig;

const KkcConfig *kkc_get_config(void);
void kkc_free_config(const KkcConfig *kkcConfigPtr);

ComposingText *kkc_get_composing_text_instance(void);
void kkc_free_composing_text_instance(ComposingText *composingTextPtr);

char *kkc_get_composing_hiragana(ComposingText *composingTextPtr);
void *kkc_free_composing_hiragana(char *hiraganaPtr);

void kkc_input_text(ComposingText *composingTextPtr, const char *stringPtr);
void kkc_delete_backward(ComposingText *composingTextPtr);
void kkc_delete_forward(ComposingText *composingTextPtr);
void kkc_complete_prefix(ComposingText *composingTextPtr,
                         int correspondingCount);

int kkc_move_cursor(ComposingText *composingTextPtr, int cursorIndex);

//
// return value: UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?
// length: candidateCounnt * 3
//
// converted text: candidate text. its ruby may be longer or shorter than input
// convertedRubyLen: length of the ruby of converted text
// subHiragana: part of whole hiragana [convertedRubyLen...]
//
// ----------------------------------------
// | index | candidate | pointee kind     |
// |-------|-----------|------------------|
// | 0     | 0         | converted text   |
// | 1     | 0         | subHiragana      |
// | 2     | 0         | convertedRubyLen |
// | 3     | 1         | converted text   |
// | 4     | 1         | subHiragana      |
// | 5     | 1         | convertedRubyLen |
// | 6     | 2         | converted text   |
// | ...   | ...       | ...              |
//
char **kkc_get_candidates(ComposingText *composingTextPtr,
                          const KkcConfig *kkcConfigPtr, bool isPredictMode,
                          int nBest);
void kkc_free_candidates(char **candidatesPtr);

#ifdef __cplusplus
}
#endif

#endif  // LIBAZOOKEY_KKC_H