#ifndef LIBAZOOKEY_KKC_H
#define LIBAZOOKEY_KKC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KkcConfig KkcConfig;
const KkcConfig *kkc_get_config(void);
void kkc_free_config(const KkcConfig *kkcConfigPtr);

struct ComposingText;
ComposingText *kkc_get_composing_text_instance(void);
void kkc_free_composing_text_instance(ComposingText *composingTextPtr);

void kkc_input_text(ComposingText *composingTextPtr, const char *stringPtr);
void kkc_delete_backward(ComposingText *composingTextPtr);
void kkc_delete_forward(ComposingText *composingTextPtr);
void kkc_complete_prefix(ComposingText *composingTextPtr,
                         int correspondingCount);

char *kkc_get_composing_hiragana(ComposingText *composingTextPtr);
char *kkc_get_composing_katakana_fullwidth(ComposingText *composingTextPtr);
char *kkc_get_composing_katakana_halfwidth(ComposingText *composingTextPtr);

// style: 0: lower, 1: upper, 2: capitalized
char *kkc_get_composing_raw_halfwidth(ComposingText *composingTextPtr,
                                      int style);
char *kkc_get_composing_raw_fullwidth(ComposingText *composingTextPtr,
                                      int style);
void *kkc_free_text(char *Ptr);

int kkc_move_cursor(ComposingText *composingTextPtr, int cursorIndex);

//
// return value:
// UnsafeMutablePointer<UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?>?
//
// candidate: candidate text shown in list
// description: description of the candidate. unused now ("")
// subHiragana: hiragana for non-converted parts
// correspondingCount: corresponding key count of the candidate
// liveTextCompatible: whether the candidate has same length with input
// Part: One of the converted phrases
// PartLen: Length of ruby of the converted phrase
//
// Example of element array in the return value array:
// ---------------------------------
// | index | value                 |
// ---------------------------------
// | 0     | candidate             |
// | 1     | description           |
// | 2     | subHiragana           |
// | 3     | correspondingCount    |
// | 4     | liveTextCompatible    |
// | 5     | Part1                 |
// | 6     | PartLen1              |
// | 7     | Part2                 |
// | 8     | PartLen2              |
// | ...   | ...                   |
//
char ***kkc_get_candidates(ComposingText *composingTextPtr,
                           const KkcConfig *kkcConfigPtr, bool isPredictMode,
                           int nBest);
void kkc_free_candidates(char ***candidatesPtr);

#ifdef __cplusplus
}
#endif

#endif  // LIBAZOOKEY_KKC_H