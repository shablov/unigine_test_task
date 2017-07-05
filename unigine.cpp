#include <algorithm>
#include <fstream>
#include <numeric>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include <chrono>
#include <iostream>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using statistics_map = std::unordered_map< std::string, std::size_t >;
using statistics_value = std::pair< std::string, std::size_t >;
using reverse_statistics_ref_value =
    std::pair< std::size_t, std::reference_wrapper< const std::string > >;

std::pair< statistics_map, statistics_map > parse_file( const std::string& input_filename )
{
    statistics_map domains;
    statistics_map paths;

    std::regex url_regex( "https?://([a-z.]+)(/{1}[a-zA-Z0-9.,/+_]+){1}.*" );

    std::ifstream input_file( input_filename );
    std::string line;
    std::smatch m;
    while ( !input_file.eof() ) {
        input_file >> line;
        if ( std::regex_match( line, m, url_regex ) ) {
            domains[ m[ 1 ] ] += 1;
            paths[ m[ 2 ] ] += 1;
        }
    }
    return std::make_pair( domains, paths );
}

int main( int argc, char* argv[] )
{
    std::size_t top_count = std::numeric_limits< std::size_t >::max();
    int i = 1;
    if ( std::strcmp( argv[ i ], "-n" ) == 0 ) {
        top_count = std::stoull( argv[ ++i ] );
        ++i;
    }
    std::string input_filename = argv[ i ];
    std::string output_filename = argv[ ++i ];

    auto statistics = parse_file( input_filename );

    std::ofstream output_file( output_filename, std::ios_base::out );
    output_file << "total urls "
                << std::accumulate( statistics.first.begin(),
                                    statistics.first.end(),
                                    0,
                                    []( const std::size_t& value, const statistics_value& pair ) {
                                        return value + pair.second;
                                    } )
                << ", domains " << statistics.first.size() << ", paths " << statistics.second.size()
                << std::endl;

    auto flip_map_to_vector = [&top_count]( const statistics_map& input,
                                            const std::size_t& top_count ) {
        std::vector< reverse_statistics_ref_value > result;
        result.reserve( input.size() );
        for ( auto&& pair : input ) {
            result.emplace_back( pair.second, std::ref( pair.first ) );
        }
        std::partial_sort(
            result.begin(),
            result.begin() + static_cast< long >( std::min( result.size(), top_count ) ),
            result.end(),
            []( const reverse_statistics_ref_value& lhs, const reverse_statistics_ref_value& rhs ) {
                bool equal = lhs.first == rhs.first;
                if ( equal ) {
                    return lhs.second.get() < rhs.second.get();
                }
                return lhs.first > rhs.first;
            } );
        return result;
    };

    auto domains = flip_map_to_vector( statistics.first, top_count );
    auto paths = flip_map_to_vector( statistics.second, top_count );
    output_file << "\ntop domains" << std::endl;

    auto write_statistics = [& out = output_file]( const reverse_statistics_ref_value& value )
    {
        out << std::get< 0 >( value ) << " " << std::get< 1 >( value ).get().c_str() << std::endl;
    };
    std::for_each( domains.begin(),
                   domains.begin() + static_cast< long >( std::min( domains.size(), top_count ) ),
                   write_statistics );

    output_file << "\ntop paths" << std::endl;
    std::for_each( paths.begin(),
                   paths.begin() + static_cast< long >( std::min( paths.size(), top_count ) ),
                   write_statistics );
    return 0;
}
