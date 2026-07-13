// The Computer Language Benchmarks Game
// http://benchmarksgame.alioth.debian.org/
//
// Contributed by Jeremy Zerfas

// This controls the maximum length for each set of oligonucleotide frequencies
// and each oligonucleotide count output by this program.
#define MAXIMUM_OUTPUT_LENGTH 4096

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint64_t key;
    uint32_t value;
    int used;
} kh_entry;

typedef struct {
    kh_entry *entries;
    size_t capacity;
    size_t size;
} khash_oligonucleotide;

typedef int khiter_t;

static khash_oligonucleotide *kh_init_oligonucleotide(void) {
    khash_oligonucleotide *table = calloc(1, sizeof(*table));
    if (!table) return NULL;
    table->capacity = 64;
    table->entries = calloc(table->capacity, sizeof(*table->entries));
    return table;
}

static void kh_destroy_oligonucleotide(khash_oligonucleotide *table) {
    free(table->entries);
    free(table);
}

static int kh_put_oligonucleotide(khash_oligonucleotide *table, uint64_t key, int *unused) {
    if (!table) return -1;
    if (table->size * 2U >= table->capacity) {
        size_t new_capacity = table->capacity * 2U;
        kh_entry *new_entries = calloc(new_capacity, sizeof(*new_entries));
        if (!new_entries) return -1;
        for (size_t i = 0; i < table->capacity; ++i) {
            if (table->entries[i].used) {
                size_t idx = table->entries[i].key % new_capacity;
                while (new_entries[idx].used) idx = (idx + 1U) % new_capacity;
                new_entries[idx] = table->entries[i];
            }
        }
        free(table->entries);
        table->entries = new_entries;
        table->capacity = new_capacity;
    }

    size_t idx = key % table->capacity;
    while (table->entries[idx].used && table->entries[idx].key != key) {
        idx = (idx + 1U) % table->capacity;
    }

    if (table->entries[idx].used) {
        *unused = 0;
        return (int)idx;
    }

    table->entries[idx].key = key;
    table->entries[idx].value = 0;
    table->entries[idx].used = 1;
    table->size++;
    *unused = 1;
    return (int)idx;
}

static khiter_t kh_get_oligonucleotide(khash_oligonucleotide *table, uint64_t key) {
    if (!table) return -1;
    size_t idx = key % table->capacity;
    while (table->entries[idx].used) {
        if (table->entries[idx].key == key) return (khiter_t)idx;
        idx = (idx + 1U) % table->capacity;
    }
    return -1;
}

#define khash_t(name) khash_##name
#define kh_init(name) kh_init_oligonucleotide()
#define kh_destroy(name, table) kh_destroy_oligonucleotide(table)
#define kh_put(name, table, key, unused) kh_put_oligonucleotide(table, key, unused)
#define kh_get(name, table, key) kh_get_oligonucleotide(table, key)
#define kh_value(table, k) ((table)->entries[(k)].value)
#define kh_size(table) ((table)->size)
#define kh_end(table) (-1)
#define kh_foreach(table, key, value, block) \
    do { \
        for (size_t __i = 0; __i < (table)->capacity; ++__i) { \
            if ((table)->entries[__i].used) { \
                uint64_t key = (table)->entries[__i].key; \
                uint32_t value = (table)->entries[__i].value; \
                block; \
            } \
        } \
    } while (0)

// intptr_t should be the native integer type on most sane systems.
typedef intptr_t intnative_t;

typedef struct {
	uint64_t	key;
	uint32_t	value;
} element;


// Macro to convert a nucleotide character to a code. Note that upper and lower
// case ASCII letters only differ in the fifth bit from the right and we only
// need the three least significant bits to differentiate the letters 'A', 'C',
// 'G', and 'T'. Spaces in this array/string will never be used as long as
// characters other than 'A', 'C', 'G', and 'T' aren't used.
#define code_For_Nucleotide(nucleotide) (" \0 \1\3  \2"[nucleotide & 0x7])


// And one more macro to convert the codes back to nucleotide characters.
#define nucleotide_For_Code(code) ("ACGT"[code & 0x3])


// Function to use when sorting elements with qsort() later. Elements with
// larger values will come first and in cases of identical values then elements
// with smaller keys will come first.
static int element_Compare(const element * const left_Element
  , const element * const right_Element){

	// Sort based on element values.
	if(left_Element->value < right_Element->value) return 1;
	if(left_Element->value > right_Element->value) return -1;

	// If we got here then both items have the same value so then sort based on
	// key.
	return left_Element->key > right_Element->key ? 1 : -1;
}


// Generate frequencies for all oligonucleotides in polynucleotide that are of
// desired_Length_For_Oligonucleotides and then save it to output.
static void generate_Frequencies_For_Desired_Length_Oligonucleotides(
  const char * const polynucleotide, const intnative_t polynucleotide_Length
  , const intnative_t desired_Length_For_Oligonucleotides, char * const output){

	khash_t(oligonucleotide) * hash_Table=kh_init(oligonucleotide);

	uint64_t key=0;
	const uint64_t mask=((uint64_t)1<<2*desired_Length_For_Oligonucleotides)-1;

	// For the first several nucleotides we only need to append them to key in
	// preparation for the insertion of complete oligonucleotides to hash_Table.
	for(intnative_t i=0; i<desired_Length_For_Oligonucleotides-1; i++)
		key=(key<<2 & mask) | polynucleotide[i];

	// Add all the complete oligonucleotides of
	// desired_Length_For_Oligonucleotides to hash_Table and update the count
	// for each oligonucleotide.
	for(intnative_t i=desired_Length_For_Oligonucleotides-1
	  ; i<polynucleotide_Length; i++){

		key=(key<<2 & mask) | polynucleotide[i];

		int element_Was_Unused;
		const khiter_t k=kh_put(oligonucleotide, hash_Table, key
		  , &element_Was_Unused);

		// If the element_Was_Unused, then initialize the count to 1, otherwise
		// increment the count.
		if(element_Was_Unused)
			kh_value(hash_Table, k)=1;
		else
			kh_value(hash_Table, k)++;
	}

	// Create an array of elements from hash_Table.
	intnative_t elements_Array_Size=kh_size(hash_Table), i=0;
	element * elements_Array=malloc(elements_Array_Size*sizeof(element));
	uint32_t value;
	kh_foreach(hash_Table, key, value
	  , elements_Array[i++]=((element){key, value}));

	kh_destroy(oligonucleotide, hash_Table);

	// Sort elements_Array.
	qsort(elements_Array, elements_Array_Size, sizeof(element)
	  , (int (*)(const void *, const void *)) element_Compare);

	// Print the frequencies for each oligonucleotide.
	for(intnative_t output_Position=0, i=0; i<elements_Array_Size; i++){

		// Convert the key for the oligonucleotide to a string.
		char oligonucleotide[desired_Length_For_Oligonucleotides+1];
		for(intnative_t j=desired_Length_For_Oligonucleotides-1; j>-1; j--){
			oligonucleotide[j]=nucleotide_For_Code(elements_Array[i].key);
			elements_Array[i].key>>=2;
		}
		oligonucleotide[desired_Length_For_Oligonucleotides]='\0';

		// Output the frequency for oligonucleotide to output.
		output_Position+=snprintf(output+output_Position
		  , MAXIMUM_OUTPUT_LENGTH-output_Position, "%s %.3f\n", oligonucleotide
		  , 100.0f*elements_Array[i].value
		  /(polynucleotide_Length-desired_Length_For_Oligonucleotides+1));
	}

	free(elements_Array);
}


// Generate a count for the number of times oligonucleotide appears in
// polynucleotide and then save it to output.
static void generate_Count_For_Oligonucleotide(
  const char * const polynucleotide, const intnative_t polynucleotide_Length
  , const char * const oligonucleotide, char * const output){
	const intnative_t oligonucleotide_Length=strlen(oligonucleotide);

	khash_t(oligonucleotide) * const hash_Table=kh_init(oligonucleotide);

	uint64_t key=0;
	const uint64_t mask=((uint64_t)1<<2*oligonucleotide_Length)-1;

	// For the first several nucleotides we only need to append them to key in
	// preparation for the insertion of complete oligonucleotides to hash_Table.
	for(intnative_t i=0; i<oligonucleotide_Length-1; i++)
		key=(key<<2 & mask) | polynucleotide[i];

	// Add all the complete oligonucleotides of oligonucleotide_Length to
	// hash_Table and update the count for each oligonucleotide.
	for(intnative_t i=oligonucleotide_Length-1; i<polynucleotide_Length; i++){

		key=(key<<2 & mask) | polynucleotide[i];

		int element_Was_Unused;
		const khiter_t k=kh_put(oligonucleotide, hash_Table, key
		  , &element_Was_Unused);

		// If the element_Was_Unused, then initialize the count to 1, otherwise
		// increment the count.
		if(element_Was_Unused)
			kh_value(hash_Table, k)=1;
		else
			kh_value(hash_Table, k)++;
	}

	// Generate the key for oligonucleotide.
	key=0;
	for(intnative_t i=0; i<oligonucleotide_Length; i++)
		key=(key<<2) | code_For_Nucleotide(oligonucleotide[i]);

	// Output the count for oligonucleotide to output.
	khiter_t k=kh_get(oligonucleotide, hash_Table, key);
	uintmax_t count=k==kh_end(hash_Table) ? 0 : kh_value(hash_Table, k);
	snprintf(output, MAXIMUM_OUTPUT_LENGTH, "%ju\t%s", count, oligonucleotide);

	kh_destroy(oligonucleotide, hash_Table);
}


int main(){
	char buffer[4096];

	// Find the start of the third polynucleotide.
	while(fgets(buffer, sizeof(buffer), stdin) && memcmp(">THREE", buffer
	  , sizeof(">THREE")-1));

	// Start with 1 MB of storage for reading in the polynucleotide and grow
	// geometrically.
	intnative_t polynucleotide_Capacity=1048576;
	intnative_t polynucleotide_Length=0;
	char * polynucleotide=malloc(polynucleotide_Capacity);

	// Start reading and encoding the third polynucleotide.
	while(fgets(buffer, sizeof(buffer), stdin) && buffer[0]!='>'){
		for(intnative_t i=0; buffer[i]!='\0'; i++)
			if(buffer[i]!='\n')
				polynucleotide[polynucleotide_Length++]
				  =code_For_Nucleotide(buffer[i]);

		// Make sure we still have enough memory allocated for any potential
		// nucleotides in the next line.
		if(polynucleotide_Capacity-polynucleotide_Length<sizeof(buffer))
			polynucleotide=realloc(polynucleotide, polynucleotide_Capacity*=2);
	}

	// Free up any leftover memory.
	polynucleotide=realloc(polynucleotide, polynucleotide_Length);

	char output_Buffer[7][MAXIMUM_OUTPUT_LENGTH];

	// Do the following functions in parallel.
	#pragma omp parallel sections
	{
		#pragma omp section
		generate_Count_For_Oligonucleotide(polynucleotide
		  , polynucleotide_Length, "GGTATTTTAATTTATAGT", output_Buffer[6]);
		#pragma omp section
		generate_Count_For_Oligonucleotide(polynucleotide
		  , polynucleotide_Length, "GGTATTTTAATT", output_Buffer[5]);
		#pragma omp section
		generate_Count_For_Oligonucleotide(polynucleotide
		  , polynucleotide_Length, "GGTATT", output_Buffer[4]);
		#pragma omp section
		generate_Count_For_Oligonucleotide(polynucleotide
		  , polynucleotide_Length, "GGTA", output_Buffer[3]);
		#pragma omp section
		generate_Count_For_Oligonucleotide(polynucleotide
		  , polynucleotide_Length, "GGT", output_Buffer[2]);

		#pragma omp section
		generate_Frequencies_For_Desired_Length_Oligonucleotides(polynucleotide
		  , polynucleotide_Length, 2, output_Buffer[1]);
		#pragma omp section
		generate_Frequencies_For_Desired_Length_Oligonucleotides(polynucleotide
		  , polynucleotide_Length, 1, output_Buffer[0]);
	}

	// Output the results to stdout.
	for(intnative_t i=0; i<7; printf("%s\n", output_Buffer[i++]));

	free(polynucleotide);

	return 0;
}
    
