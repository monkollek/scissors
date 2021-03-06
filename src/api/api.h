#include "dataStructures/target_event.h"
#include "dataStructures/target_region.h"
#include "outsources/fasta/Fasta.h"
#include "outsources/samtools/bam.h"
#include "utilities/miscellaneous/aligner.h"
#include "utilities/miscellaneous/alignment_filter.h"
#include "utilities/hashTable/reference_hasher.h"

// Need for special insertion detection
#include "utilities/hashTable/special_hasher.h"
