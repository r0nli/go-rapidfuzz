package rapidfuzz

/*
#cgo CFLAGS: -Iinclude

#cgo linux,amd64 LDFLAGS: -L${SRCDIR}/libs -static -lextract-linux-amd64 -lstdc++ -lm
#cgo windows,amd64 LDFLAGS: -L${SRCDIR}/libs -static -lextract-windows-amd64 -lstdc++ -lm

#include <stdlib.h>

// Define the ExtractResult and ExtractResultsArray structs to match C++
typedef struct {
    int index;
    const char* match;
    double score;
} ExtractMatch;

typedef struct {
    ExtractMatch* matches;
    int size;
} ExtractResultsArray;

typedef struct {
    const char* best_match;
    double best_score;
    int found;
} ExtractResult;

ExtractResult extract_one_c(const char* query, const char** choices, int num_choices, double score_cutoff);
ExtractResultsArray extract_c(const char *query, const char **choices, int num_choices, double score_cutoff);
void free_extract_result(ExtractResult result);
void free_extract_results_array(ExtractResultsArray results);
*/
import "C"
import (
	"unsafe"
)

type Match struct {
	Index int
	Match string
	Score float64
}

// Go function to call extractOne via CGO
func extractOne(query string, choices []string, scoreCutoff float64) (string, float64, bool) {
	cQuery := C.CString(query)
	defer C.free(unsafe.Pointer(cQuery))

	// Convert Go strings to C strings for the choices array
	cChoices := make([]*C.char, len(choices))
	for i, choice := range choices {
		cChoices[i] = C.CString(choice)
		defer C.free(unsafe.Pointer(cChoices[i])) // Free each C string after use
	}

	// Call the C++ function
	result := C.extract_one_c(cQuery, &cChoices[0], C.int(len(choices)), C.double(scoreCutoff))
	defer C.free_extract_result(result) // Free the result's allocated memory

	// Check if a match was found
	if result.found == 0 {
		return "", 0.0, false
	}

	// Convert the C string result.best_match to a Go string
	bestMatch := C.GoString(result.best_match)
	bestScore := float64(result.best_score)

	return bestMatch, bestScore, true
}

// Go function to call extract via CGO
func extract(query string, choices []string, scoreCutoff float64) []Match {
	cQuery := C.CString(query)
	defer C.free(unsafe.Pointer(cQuery))

	cChoices := make([]*C.char, len(choices))
	for i, choice := range choices {
		cChoices[i] = C.CString(choice)
		defer C.free(unsafe.Pointer(cChoices[i]))
	}

	results := C.extract_c(cQuery, &cChoices[0], C.int(len(choices)), C.double(scoreCutoff))
	defer C.free_extract_results_array(results)

	goResults := make([]Match, results.size)

	matches := (*[1 << 30]C.ExtractMatch)(unsafe.Pointer(results.matches))[:results.size:results.size]

	for i := 0; i < int(results.size); i++ {
		index := int(matches[i].index)        // Retrieve the index
		match := C.GoString(matches[i].match) // Retrieve the match string
		score := float64(matches[i].score)    // Retrieve the score
		goResults[i] = Match{Index: index, Match: match, Score: score}
	}

	return goResults
}

func ExtractOne(query string, choices []string, scoreCutoff float64) (string, float64, bool) {
	return extractOne(query, choices, scoreCutoff)
}

func Extract(query string, choices []string, scoreCutoff float64) []Match {
	return extract(query, choices, scoreCutoff)
}
