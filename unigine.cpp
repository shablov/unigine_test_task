#include <algorithm>
#include <fstream>
#include <iterator>
#include <map>
#include <regex>
#include <string>
#include <vector>

auto get_domain(const std::string &url) {
	return url;
}

auto get_path(const std::string &url) {
	return url;
}

auto read_line(std::ifstream& input_file) {
    std::string tmp;
    input_file >> tmp;
    return "https://en.wikipedia.org/w/index.php?title=Kirschkuchen&action=edit&section=8";
}

auto parse_file(const std::string& input_filename) {
	std::ifstream input_file(input_filename);

	std::map<std::string, int> domains;
	std::map<std::string, int> paths;

	std::regex url_regex("(https?://){1}(.*)\\s");
	while (!input_file.eof()) {
		std::string line = read_line(input_file);

    	std::smatch m;
    	std::regex_search(line, m, url_regex);
    	for (auto &&url : m) {
    		domains[get_domain(url)] += 1;
    		paths[get_path(url)] += 1;
    	}
	}
	return std::make_pair(domains, paths);
}
using statistics_value = std::pair<std::string, int>;
using reverse_statistics_value = std::tuple<std::reference_wrapper<const int>, std::reference_wrapper<const std::string>>;

int main(int argc, char *argv[])
{
	uint64_t top_count = std::numeric_limits<uint64_t>::max();
	int i = 1;
	if (argv[i] == "-n") {
		top_count = std::stoull(argv[++i]);
		++i;
	}
	std::string input_filename = argv[i];
	std::string output_filename = argv[++i];

    auto statistics = parse_file(input_filename);

    std::ofstream output_file(output_filename, std::ios_base::out);
    output_file << "total urls " <<  std::accumulate(statistics.first.begin(), statistics.first.end(),
                                                     0, [] (const uint64_t& value, const statistics_value& pair) {
                                                     	return value + pair.second;
                                                     })
                << ", domains " << statistics.first.size()
                << ", paths " << statistics.second.size() << std::endl;

    auto flip_map_to_vector = [&top_count] (const std::map<std::string, int> input, const uint64_t& top_count) { 
    	std::vector<reverse_statistics_value> result;
    	for (auto &&pair : input) {
    		result.push_back(std::make_tuple(std::ref(pair.second), std::ref(pair.first)));
    	}
    	std::partial_sort(result.begin(), result.begin() + std::min(result.size(), top_count), result.end(),
    	                  [] (const reverse_statistics_value& lhs, const reverse_statistics_value& rhs) {
    	                  	return std::get<0>(lhs) > std::get<0>(rhs) && std::get<1>(lhs).get() > std::get<1>(rhs).get();
    	                  });
    	return result;
    };

    auto domains = flip_map_to_vector(statistics.first, top_count);
    auto paths = flip_map_to_vector(statistics.second, top_count);
    output_file << "\ntop domains" << std::endl;
    
    auto write_statistics = [&output_file] (const reverse_statistics_value& value) {
                                output_file << std::get<0>(value) << " " << std::get<1>(value).get().c_str() << std::endl;
                            };
    std::for_each(domains.begin(), domains.begin() + std::min(domains.size(), top_count), write_statistics);
                    

    output_file << "\ntop paths" << std::endl;
    std::for_each(paths.begin(), paths.begin() + std::min(paths.size(), top_count), write_statistics);

    return 0;
}