/*******************************************************************************
 *   Copyright (C) 2014, 2015 Giles Miclotte (giles.miclotte@intec.ugent.be)   *
 *   This file is part of Jabba                                                *
 *                                                                             *
 *   This program is free software; you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published by      *
 *   the Free Software Foundation; either version 2 of the License, or         *
 *   (at your option) any later version.                                       *
 *                                                                             *
 *   This program is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 *   GNU General Public License for more details.                              *
 *                                                                             *
 *   You should have received a copy of the GNU General Public License         *
 *   along with this program; if not, write to the                             *
 *   Free Software Foundation, Inc.,                                           *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 *
 *******************************************************************************/
#include "SeedFinder.hpp"

#include <sstream>
#include <iostream>
#include <fstream>

#include "Sequence.hpp"
#include "Seed.hpp"
#include "mummer/sparseSA.hpp"
#include "Input.hpp"

SeedFinder::SeedFinder(std::string const &dir, std::string const &graph_name, int min_length, int k) {
	init(dir, graph_name, min_length, k);
}

void SeedFinder::init (std::string const &dir, std::string const &graph_name, int min_length, int k) {
	graph_name_ = graph_name;
	min_length_ = min_length;
	preprocessGraph(reference_);
	sa_ = init_essaMEM(reference_, dir + "/" + Input::parseBasename(graph_name_), k);
}

SeedFinder::~SeedFinder(){
	delete sa_;
}

void SeedFinder::preprocessGraph(std::string &reference) {
	//file to read from
	std::ifstream graph((graph_name_).c_str());
	std::cout << "Seedfinder: reading graph from file \"" << graph_name_
		<< "\"." << std::endl;
	std::vector<std::string> nodes;
	std::string node_number;
	std::string node;
	std::string in_edges;
	std::string out_edges;
	
	nodes_index_.push_back(0);
	
	int signed current_node = 1;
	int total_size = 0;
	if (graph.is_open()){
		while (graph){
			getline(graph, node_number);
			if(!graph){
				break;
			}
			getline(graph, node);
			getline(graph, in_edges);
			getline(graph, out_edges);
			
			reference += node + "#";
			total_size += node.size() + 1;
			nodes_index_.push_back(total_size);
			
			reference += Sequence::reverseComplement(node) + "#";
			total_size += node.size() + 1;
			nodes_index_.push_back(total_size);
			
			current_node++;
		}
	} else {
		std::cerr << "Error: file is not open." << std::endl;
		return;
	}
	graph.close();
	std::cout << "Done.";
}

int SeedFinder::binary_node_search(int const &mem_start) {
	int signed left = 0;
	int signed mid;
	int signed right = nodes_index_.size();
	while(left + 1 != right){
		mid = (left + right) / 2;
		if(nodes_index_.at(mid) < mem_start){
			left = mid;
		} else if(nodes_index_.at(mid) > mem_start){
			right = mid;
		} else {
			left = mid;
			right = mid + 1;
		}
	}
	return (((left + 2) / 2)) * (left % 2 == 0 ? 1 : -1);
}

int SeedFinder::startOfHit(int node_nr, int start_in_ref) {
	
	int start_of_node = nodes_index_.at(2 * node_nr * (node_nr < 0 ? -1 : 1) - 2 + (node_nr < 0));
	return start_in_ref - start_of_node;
}
void SeedFinder::getSeeds(
	std::string read,
	std::map<int, std::vector<Seed>> &seed_map,
	std::vector<int> &seeds_of_size,
	std::vector<int> &map_keys,
	int &seed_count,
	int const &seed_min_length
){
	//mem finding
	vector<match_t> matches;	//will contain the matches
	std::string query = read;
	bool print = 0;	//not sure what it prints if set to 1
	long memCounter = 0;	//this does not really seem to work
	int num_threads = 1;
	int forward = true;
	sa_->MEM(query, matches, seed_min_length, print, memCounter, forward, num_threads);
		
	//parse the results
	for (int i = 0; i < matches.size(); ++i) {
		match_t m = matches[i];
		int node_nr = binary_node_search(m.ref);
		int node_start = startOfHit(node_nr, m.ref);
		seed_map[node_nr].push_back(Seed(node_nr, node_start, m.query, m.len));
		while (seeds_of_size.size() <= m.len) {
			seeds_of_size.push_back(0);
		}
		++seeds_of_size[m.len];
		++seed_count;
		if (std::find(map_keys.begin(), map_keys.end(), node_nr) == map_keys.end()) {
			map_keys.push_back(node_nr);
		}
	}
}

sparseSA * SeedFinder::init_essaMEM(std::string &ref, std::string const &meta,
	int k)
{
	std::vector<std::string> refdescr;
	refdescr.push_back(meta);
	std::vector<long> startpos;
	startpos.push_back(0); //only one reference
	bool printSubstring = false;
	bool printRevCompForw = false;
	int sparseMult = 1;
	bool suflink = true;
	bool child = false;
	sparseSA * sa;
	sa = new sparseSA(ref,
		refdescr,
		startpos,
		false,
		k,
		suflink,
		child,
		false,
		sparseMult,
		0,
		printSubstring,
		printRevCompForw,
		false
	);
	sa->construct();
	
	stringstream * prefixstream = new stringstream();
	(*prefixstream) << meta << "_" << k << "_" << suflink << "_" << child;
	string prefix = prefixstream->str();
	if(!sa->load(prefix)) {
		sa->construct();
		sa->save(prefix);
	}
	delete prefixstream;
	cerr << "INDEX SIZE IN BYTES: " << sa->index_size_in_bytes() << endl;
	return sa;
}

