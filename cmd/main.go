package main

import (
	"fmt"

	fuzz "github.com/r0nli/go-rapidfuzz"
)

func main() {
	query := "hello world"
	choices := []string{"hi there", "hello", "world", "hello world", "hello wo", "helllo world"}
	scoreCutoff := 70.0

	bestMatch, bestScore, found := fuzz.ExtractOne(query, choices, scoreCutoff)
	if found {
		fmt.Printf("Best match: %s with score: %.2f\n", bestMatch, bestScore)
	} else {
		fmt.Println("No match found above the score cutoff.")
	}

	matches := fuzz.Extract(query, choices, scoreCutoff)
	fmt.Println("All matches from extract:")
	for _, match := range matches {
		fmt.Printf("Index: %d, Match: %s, Score: %.2f\n", match.Index, match.Match, match.Score)
	}
}
