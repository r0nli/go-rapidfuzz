#include <string>
#include <vector>
#include <optional>
#include <utility>
#include <cstring>
#include <rapidfuzz/fuzz.hpp>

template <typename Sentence1,
          typename Iterable, typename Sentence2 = typename Iterable::value_type>
std::optional<std::pair<Sentence2, double>>
extractOne(const Sentence1 &query, const Iterable &choices, const double score_cutoff = 0.0)
{
  bool match_found = false;
  double best_score = score_cutoff;
  Sentence2 best_match;

  rapidfuzz::fuzz::CachedRatio<typename Sentence1::value_type> scorer(query);

  for (const auto &choice : choices)
  {
    double score = scorer.similarity(choice, best_score);

    if (score >= best_score)
    {
      match_found = true;
      best_score = score;
      best_match = choice;
    }
  }

  if (!match_found)
  {
    return std::nullopt;
  }

  return std::make_pair(best_match, best_score);
}

template <typename Sentence1,
          typename Iterable, typename Sentence2 = typename Iterable::value_type>
std::vector<std::tuple<size_t, Sentence2, double>>
extract(const Sentence1 &query, const Iterable &choices, const double score_cutoff = 0.0)
{
  std::vector<std::tuple<size_t, Sentence2, double>> results;

  rapidfuzz::fuzz::CachedRatio<typename Sentence1::value_type> scorer(query);

  size_t index = 0;
  for (const auto &choice : choices)
  {
    double score = scorer.similarity(choice, score_cutoff);

    if (score >= score_cutoff)
    {
      results.emplace_back(index, choice, score);
    }
    ++index;
  }

  return results;
}

extern "C"
{

  // Define a struct for individual match results to be returned to Go
  struct ExtractMatch
  {
    int index; // Add index of the match
    const char *match;
    double score;
  };

  // Define a struct to hold an array of results for Go to process
  struct ExtractResultsArray
  {
    ExtractMatch *matches;
    int size;
  };

  // Define a struct for the result to be returned to Go
  struct ExtractResult
  {
    const char *best_match;
    double best_score;
    bool found;
  };
  // Wrapper for the extractOne function
  ExtractResult extract_one_c(const char *query, const char **choices, int num_choices, double score_cutoff)
  {
    std::string query_str(query);
    std::vector<std::string> choices_vec;

    // Copy the C-style array of choices into a std::vector of strings
    for (int i = 0; i < num_choices; i++)
    {
      choices_vec.emplace_back(choices[i]);
    }

    // Call the original extractOne function
    auto result = extractOne(query_str, choices_vec, score_cutoff);

    // Prepare the result to return to Go
    ExtractResult output;
    if (result)
    {
      // Allocate memory for the best match string to return
      char *best_match = new char[result->first.size() + 1];
      std::strcpy(best_match, result->first.c_str());
      output.best_match = best_match;
      output.best_score = result->second;
      output.found = true;
    }
    else
    {
      output.best_match = nullptr;
      output.best_score = 0.0;
      output.found = false;
    }
    return output;
  }

  // New wrapper function for extract, which returns multiple matches
  ExtractResultsArray extract_c(const char *query, const char **choices, int num_choices, double score_cutoff)
  {
    std::string query_str(query);
    std::vector<std::string> choices_vec;

    for (int i = 0; i < num_choices; i++)
    {
      choices_vec.emplace_back(choices[i]);
    }

    auto results = extract(query_str, choices_vec, score_cutoff);

    // Allocate memory for the results to be returned to Go
    ExtractMatch *matches = new ExtractMatch[results.size()];
    for (size_t i = 0; i < results.size(); i++)
    {
      const auto &[index, choice, score] = results[i];
      char *match = new char[choice.size() + 1];
      std::strcpy(match, choice.c_str());
      matches[i].index = index; // Set the index
      matches[i].match = match; // Set the matched string
      matches[i].score = score; // Set the similarity score
    }

    // Create and return the array of matches
    ExtractResultsArray output;
    output.matches = matches;
    output.size = results.size();
    return output;
  }

  // Function to free the memory allocated for the array of results
  void free_extract_results_array(ExtractResultsArray results)
  {
    for (int i = 0; i < results.size; i++)
    {
      delete[] results.matches[i].match;
    }
    delete[] results.matches;
  }

  // Function to free the memory allocated for the best_match string
  void free_extract_result(ExtractResult result)
  {
    if (result.best_match)
    {
      delete[] result.best_match;
    }
  }
}