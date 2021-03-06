#include <list>
#include <string>
#include <iostream>
#include <outsources/samtools/bam.h>

using std::string;

namespace Scissors {

namespace {
inline void ExtenedBamDataBuffer(
    const int& needed_size, 
    bam1_t* bam_record) {
  bam_record->m_data = needed_size;
  kroundup32(bam_record->m_data);
  uint8_t* ptr = bam_record->data;
  bam_record->data = (uint8_t*)calloc(bam_record->m_data, sizeof(uint8_t));
  memcpy(bam_record->data, ptr, bam_record->data_len);
  free(ptr);
}
} // unmaed namespace

namespace OptionalTag {
void AddOptionalTags(const bam1_t& anchor, bam1_t* alignment) {
  const uint8_t* rg_ptr = bam_aux_get(&anchor, "RG");
  if (rg_ptr == NULL) return;
  const char* rg = bam_aux2Z(rg_ptr);
  int rg_len = strlen(rg);

  int ori_len = alignment->data_len;
  alignment->data_len += (3 + rg_len + 1);
  alignment->l_aux += (3 + rg_len + 1);
  if (alignment->m_data < alignment->data_len)
    ExtenedBamDataBuffer(alignment->data_len, alignment);

  alignment->data[ori_len] = 'R';
  alignment->data[ori_len + 1] = 'G';
  alignment->data[ori_len + 2] = 'Z';
  memcpy(alignment->data + ori_len + 3, rg, rg_len);
  alignment->data[ori_len + 3 + rg_len] = '\0';
}

void AddOptionalTags(const bam1_t& anchor, std::list<bam1_t*> alignments) {
  for (std::list<bam1_t*>::iterator ite = alignments.begin();
       ite != alignments.end(); ++ite) {
    AddOptionalTags(anchor, *ite);
  }
}
} // namespace OptionalTags
} // namespace Scissors
