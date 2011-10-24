#include "aligner.h"

#include "utilities/miscellaneous/hashes_collection.h"

void SetTargetSequence(const SearchRegionType::RegionType& region_type, 
                       SR_QueryRegion* query_region) {
  const bool forward    = !bam1_strand(query_region->pOrphan);
  const bool inverse    = (forward && (query_region->isOrphanInversed == TRUE))
                        ||(!forward && (query_region->isOrphanInversed == FALSE));
  const bool complement = !forward;
  
  if (region_type.sequence_inverse && region_type.sequence_complement) {
    if (!inverse && !complement)
      SR_QueryRegionChangeSeq(query_region, SR_REVERSE_COMP);
    else if (!inverse)
      SR_QueryRegionChangeSeq(query_region, SR_INVERSE);
    else if (!complement)
      SR_QueryRegionChangeSeq(query_region, SR_COMP);

    SR_SetStrand(query_region->pOrphan, SR_REVERSE_COMP);
    query_region->isOrphanInversed = FALSE;
    
  } else if (region_type.sequence_inverse && !region_type.sequence_complement) {
    if (!inverse && complement)
      SR_QueryRegionChangeSeq(query_region, SR_REVERSE_COMP);
    else if (!inverse)
      SR_QueryRegionChangeSeq(query_region, SR_INVERSE);
    else if (complement)
      SR_QueryRegionChangeSeq(query_region, SR_COMP);
      
    SR_SetStrand(query_region->pOrphan, SR_INVERSE);
    query_region->isOrphanInversed = TRUE;
  
  } else if (!region_type.sequence_inverse && region_type.sequence_complement) {
    if (inverse && !complement)
      SR_QueryRegionChangeSeq(query_region, SR_REVERSE_COMP);
    else if (inverse)
      SR_QueryRegionChangeSeq(query_region, SR_INVERSE);
    else if (!complement)
      SR_QueryRegionChangeSeq(query_region, SR_COMP);

    SR_SetStrand(query_region->pOrphan, SR_REVERSE_COMP);
    query_region->isOrphanInversed = TRUE;
  
  } else {
    if (inverse && complement)
      SR_QueryRegionChangeSeq(query_region, SR_REVERSE_COMP);
    else if (inverse)
      SR_QueryRegionChangeSeq(query_region, SR_INVERSE);
    else if (complement)
      SR_QueryRegionChangeSeq(query_region, SR_COMP);

    SR_SetStrand(query_region->pOrphan, SR_FORWARD);
    query_region->isOrphanInversed = FALSE;
  } // end if-else
}

Aligner::Aligner(const SR_Reference* reference, 
                 const SR_InHashTable* hash_table) 
    : reference_(reference)
    , hash_table_(hash_table) {
  query_region_ = SR_QueryRegionAlloc();
  hashes_       = HashRegionTableAlloc();

  hash_length_.fragLen = 1000;
  hash_length_.closeRange = 2000;
  hash_length_.farRange   = 10000;
}

Aligner::~Aligner() {
  SR_QueryRegionFree(query_region_);
  HashRegionTableFree(hashes_);
}

void Aligner::LoadRegionType(const bam1_t& anchor) {
  uint32_t* cigar = bam1_cigar(&anchor);
  bool is_new_region = anchor_region_.IsNewRegion(cigar,
                                                  anchor.core.n_cigar, 
                                                  anchor.core.pos);
  
  if (is_new_region)
    search_region_type_.ResetRegionTypeList();
  else
    search_region_type_.RewindRegionTypeList();
}


void Aligner::AlignCandidate(SR_BamListIter* al_ite,
                             vector<bam1_t>* alignments) {

    while (SR_QueryRegionLoadPair(query_region_, al_ite) == SR_OK) {
      const bool is_anchor_forward = !bam1_strand(query_region_->pAnchor);
      // Convert 4-bit representive sequence into chars
      SR_QueryRegionLoadSeq(query_region_);
      int read_length = query_region_->pOrphan->core.l_qseq;

      LoadRegionType(*(query_region_->pAnchor));

      SearchRegionType::RegionType region_type;
      HashesCollection hashes_collection;
      while (search_region_type_.GetNextRegionType(is_anchor_forward, 
                                                   &region_type)) {
        // Reverse or complement the sequence if necesary
        SetTargetSequence(region_type, query_region_);
        HashRegionTableInit(hashes_, read_length);
        SR_QueryRegionSetRange(query_region_, &hash_length_, reference_->seqLen,
                               region_type.upstream ? SR_UPSTREAM : SR_DOWNSTREAM);
	HashRegionTableLoad(hashes_, hash_table_, query_region_);
	hashes_collection.Init(*(hashes_->pBestCloseRegions));
	hashes_collection.SortByLength();

	int hashes_count = hashes_collection.GetSize();
	if (hashes_count == 0) continue;
	if ((hashes_collection.Get(hashes_count-1))->numPos != 0) {
	  //BamAlignment al;
	  //al.query_name = bam1_qname(query_region_->pAnchor);
	  //al.reference_index = query_region_->pAnchor->core.tid;
	  //al.reference_begin = (hashes_collection.Get(hashes_count-1))->refBegins[0];
	  //al.reference_end   = (hashes_collection.Get(hashes_count-1))->refBegins[0] + (hashes_collection.Get(hashes_count-1))->length - 1;
	  //alignments->push_back(al);
	}

	//bam_write1( files.bam_writer, vars.query_region->pAnchor );
	//bam_write1( files.bam_writer, vars.query_region->pOrphan );
	//
      } // end while
    } // end while

    al_ite = NULL;
}