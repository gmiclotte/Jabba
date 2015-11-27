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
#ifndef SEEDFINDER_HPP
#define SEEDFINDER_HPP

#include <vector>
#include <string>
#include <map>
#include <iostream>

class sparseSA;
class Seed;

class SeedFinder{
	private:
		int min_length_; //min length of seeds
		int k_; //sparseness factor
		sparseSA * sa_; //suffix array
		std::string reference_; //sparseSA requires the sequence from
					//which it is built to be kept in memory
		std::vector<int> nodes_index_; //list containing size of nodes
	public:
		/*
		 *	ctors
		 */
		SeedFinder(){nodes_index_.push_back(0);}
		SeedFinder(int min_length, int k);
		void init();
		/*
		 *	dtors
		 */
		~SeedFinder();
		/*
		 *	methods
		 */
		void set_min_length(int min_length) {min_length_ = min_length;}
		void set_k(int k) {k_ = k;}
		void addNodeToReference(std::string const &node);
		void preprocessReference();
		//initialise the ESSA
		sparseSA * init_essaMEM(std::string const &meta);
		//find seeds between read and the graph
		void getSeeds(std::string const &read,
			std::map<int, std::vector<Seed>> &seed_map,
			std::vector<int> &seeds_of_size,
			std::vector<int> &map_keys, int &seed_count,
			int const &seed_min_length) const;
		//find the node in which a seed is contained
		int binary_node_search(int const &mem_start) const;
		//find where in the node the seed starts
		int startOfHit(int node_nr, int start_in_ref) const;
		//
		std::string getNode(int const node_id) const {
			int index = 2 * node_id * (node_id < 0 ? -1 : 1) - 2 + (node_id < 0);
			int pos = nodes_index_[index];
			int len = nodes_index_[index + 1] - pos - 1;
			return reference_.substr(pos, len);
		}
};
#endif
