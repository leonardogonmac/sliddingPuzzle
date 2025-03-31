#include <bits/stdc++.h>
#include <sys/resource.h>
#include "boost/multi_array.hpp"
using namespace std;

#define HASH 3
#define SIZE 16

typedef enum direction {UP, DOWN, LEFT, RIGHT} direction;
typedef enum Plist {OPEN, CLOSED} Plist;


typedef struct piece {
	int num;
	pair<int, int> goal;
	
	bool operator==(piece x){
		if(x.num == num)
			return true;
		else
			return false;
	}
	bool operator!=(piece x){
		if(x.num == num)
			return false;
		else
			return true;
	}
}piece; 

typedef struct puzzle {
	int steps;
	vector<vector<piece>> m;
	pair<int, int> empty;
	array<int, SIZE> coord;
	int ind;
	bool operator==(puzzle x){
		for(int i = 0; i < m.size(); i++){
			for(int j = 0; j < m.size(); j++){
				if(x.m[i][j] != m[i][j])
					return false;
			}
		}
		return true;
	}
	bool operator!=(puzzle x){
		for(int i = 0; i < m.size(); i++){
			for(int j = 0; j < m.size(); j++){
				if(x.m[i][j] != m[i][j])
					return true;
			}
		}
		return false;
	}

	bool operator<(const puzzle& other) const {
		if(other.steps > steps)
			return true;
		else return false;
	}
}puzzle;

typedef boost::multi_array<vector<puzzle>, SIZE> puzzle_map;
typedef puzzle_map::index Pindex;


int manhattan(pair<int, int> coord1, pair<int, int> coord2){
	return abs(coord1.first - coord2.first) + abs(coord1.second - coord2.second);
}

int weight(puzzle p){
	int w = 0;
	for(int i = 0; i < p.m.size(); i++){
		for(int j = 0; j < p.m.size(); j++){
			if(p.m[i][j].num == 0)
				continue;
			w += manhattan({i, j}, p.m[i][j].goal)/**p.m[i][j].num*/;
		}
	}
	return w;
}

int find_in_row(puzzle p, vector<puzzle>& v){
	for(int i = 0; i < v.size(); i++){
		if(p == v[i])
			return i;
	}
	return -1;
}

int sum_row(puzzle p, int row){
	int sum = 0;
	for(int i = 0; i < p.m[0].size(); i++)
		sum += p.m[row][i].num;
	
	return sum;
}

int sum_column(puzzle p, int column){
	int sum = 0;
	for(int i = 0; i < p.m[0].size(); i++)
		sum += p.m[i][column].num;
	
	return sum;
}
//array<Pindex, SIZE> arr = {{HASH,HASH,HASH,HASH,HASH,HASH,HASH,HASH,HASH,HASH,HASH,HASH,HASH,HASH,HASH,HASH}};
boost::multi_array<vector<puzzle>, SIZE>* openList;
boost::multi_array<vector<puzzle>, SIZE>* closedList;

typedef struct puzzle_table{
	Plist l;
	int rank;
	puzzle_table(Plist x, int y): l(x), rank(y){
	}
	int find(puzzle p){
		array<int, SIZE> coord = hash(p);
		vector<puzzle> v;
		if(l == OPEN)
			v = (*openList)(coord);
		else 
			v = (*closedList)(coord);
		return find_in_row(p, v);
	}
	void push_back(puzzle& p){
		array<int, SIZE> coord = hash(p);
		vector<puzzle> v;
		if(l == OPEN)
			(*openList)(coord).push_back(p);
		else 
			(*closedList)(coord).push_back(p);
	}
	void erase(puzzle p){
		array<int, SIZE> coord = hash(p);
		if(l == OPEN)
			(*openList)(coord).erase((*openList)(coord).begin() + find_in_row(p, (*openList)(coord)));
		else 
			(*closedList)(coord).erase((*closedList)(coord).begin() + find_in_row(p, (*closedList)(coord)));
	}

	array<int, SIZE> hash(puzzle p){
		array<int, SIZE> coord;
		/*coord[0] = weight(p) % HASH;
		coord[1] = p.steps % HASH;
		coord[2] = sum_row(p, 0) % HASH;
		coord[3] = sum_column(p, 0) % HASH;
		*/
		int aux = 0;
		for(int i = 0; i < rank; i++){	
			for(int j = 0; j < rank; j++){
				coord[aux] = p.m[i][j].num % HASH;
				aux++;
			}
		}
		return coord;
	}
	puzzle get(array<int, SIZE> coord, int ind){
		vector<puzzle> v;
		if(l == OPEN)
			v = (*openList)(coord);
		else 
			v = (*closedList)(coord);
		return v.at(ind);
	}

}puzzle_table;

puzzle slide(puzzle p, direction d){
	puzzle new_p = p;
	int i, j, hori = 0, vert = 0;
	switch(d){
        case UP:
            vert = -1;
            break;
        case DOWN:
            vert = 1;
            break;
        case LEFT:
            hori = -1;
            break;
        case RIGHT:
            hori = 1;
    }
    tie(i, j) = new_p.empty;
    piece tmp = new_p.m[i][j];
	new_p.m[i][j] = new_p.m[i + vert][j + hori];
	new_p.m[i + vert][j + hori] = tmp;
	new_p.empty = {i + vert, j + hori};
	new_p.steps++;
    return new_p;

}

vector<puzzle> get_adj(puzzle p){
	vector<puzzle> adj;
	int i, j;
	tie(i, j) = p.empty;
	if(i > 0)
		adj.push_back(slide(p, UP));
	if(i < p.m.size() - 1)
		adj.push_back(slide(p, DOWN));
	if(j > 0)
		adj.push_back(slide(p, LEFT));
	if(j < p.m.size() - 1)
		adj.push_back(slide(p, RIGHT));
	
	return adj;
}

void print_puzzle(puzzle p){
	
	cout << "======STEP " << p.steps << "=========\n";
	cout << "WEIGHT + STEPS = " << weight(p) + p.steps << "\n\n";
	for(int i = 0; i < p.m.size(); i++){
		for(int j = 0; j < p.m.size(); j++){
			cout << p.m[i][j].num << '\t';
		}
		cout << "\n\n";
	}
}

void backtrack(puzzle p, puzzle_table& closed){
	if(p.ind == -1){
		print_puzzle(p);
		return;
	}
	puzzle parent = closed.get(p.coord, p.ind);
	backtrack(parent, closed);
	print_puzzle(p);
}

void astar(puzzle begin){
	cout << "begin astar\n";
	puzzle_table closed(CLOSED, begin.m.size());
	puzzle_table open(OPEN, begin.m.size());
	open.push_back(begin);
	priority_queue<pair<int, puzzle>> pq;
	pq.push({- (weight(begin) + begin.steps), begin});
	while(true){
		puzzle current = pq.top().second;
		pq.pop();
		closed.push_back(current);
		open.erase(current);
		vector<puzzle> adj = get_adj(current);
		
		for(int i = 0; i < adj.size(); i++){
			adj[i].coord = closed.hash(current);
			array<int, SIZE> coord = adj[i].coord;
			adj[i].ind = (*closedList)(coord).size() - 1;
			if(weight(adj[i]) == 0){
				backtrack(adj[i], closed);
				return;
			}
		}

		for(int i = 0; i < adj.size(); i++){
			if(closed.find(adj[i]) == -1 && open.find(adj[i]) == -1){
				open.push_back(adj[i]);
				pq.push({-(weight(adj[i]) + adj[i].steps), adj[i]});
			}
		}
	}
}


puzzle begin_puzzle(int rank){
	puzzle begin;
	for(int i = 0; i < rank; i++){
		vector<piece> row;
		for(int j = 0; j < rank; j++){
			piece x;
			cin >> x.num;
			if(x.num != 0)
				x.goal = {(x.num - 1)/rank, (x.num - 1) % rank};
			else{
				x.goal = {rank - 1, rank - 1};
				begin.empty = {i, j};
			}
			row.push_back(x);
		}
		begin.m.push_back(row);
	}
	begin.steps = 0;
	begin.ind = -1;
	return begin;
}

int main(){
	array<Pindex, SIZE> arr;
	for(int i = 0; i < SIZE; i++){
		arr[i] = HASH;
	}
	openList = new puzzle_map(arr);
	closedList = new puzzle_map(arr);
	int rank;
	cin >> rank;
	puzzle begin = begin_puzzle(rank);
	astar(begin);
	return 0;
}