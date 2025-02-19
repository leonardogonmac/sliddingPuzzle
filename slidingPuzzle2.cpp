#include <bits/stdc++.h>
using namespace std;

typedef enum direction {UP, DOWN, LEFT, RIGHT} direction;

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
	int parenti, parentj;
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

int manhattan(pair<int, int> coord1, pair<int, int> coord2){
	return abs(coord1.first - coord2.first) + abs(coord1.second - coord2.second);
}

int weight(puzzle p){
	int w = 0;
	for(int i = 0; i < p.m.size(); i++){
		for(int j = 0; j < p.m.size(); j++){
			w += manhattan({i, j}, p.m[i][j].goal)*p.m[i][j].num;
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

typedef struct puzzle_table{
	vector<vector<puzzle>> pt;
	int size;
	puzzle_table(int s){
		cout << "abb";
		pt.resize(s);
		size = s;
	}

	int find(puzzle p){
		int row = hash(p);
		return find_in_row(p, pt[row]);
	}
	void push_back(puzzle p){
		int row = hash(p);
		pt[row].push_back(p);
	}
	bool empty(){
		for(int i = 0; i < pt.size(); i++){
			if(!pt[i].empty())
				return false;
		}
		return true;
	}
	void erase(puzzle p){
		int row = hash(p);
		pt[row].erase(pt[row].begin() + find_in_row(p, pt[row]));
	}

	int hash(puzzle p){
		return (weight(p)) % size;
	}
	puzzle get(int i, int j){
		return pt[i].at(j);
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
	if(p.parenti == -1){
		print_puzzle(p);
		return;
	}
	puzzle parent = closed.get(p.parenti, p.parentj);
	backtrack(parent, closed);
	print_puzzle(p);
}

void astar(puzzle begin){
	cout << "begin astar\n";
	puzzle_table closed(100);
	puzzle_table open(100);
	open.push_back(begin);
	priority_queue<pair<int, puzzle>> pq;
	pq.push({- (weight(begin) + begin.steps), begin});
	while(!open.empty()){
		puzzle current = pq.top().second;
		pq.pop();
		closed.push_back(current);
		open.erase(current);
		vector<puzzle> adj = get_adj(current);
		
		for(int i = 0; i < adj.size(); i++){
			adj[i].parenti = closed.hash(current);
			adj[i].parentj = closed.pt[adj[i].parenti].size() - 1;
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
	begin.parenti = begin.parentj = -1;
	return begin;
}

int main(){
	int rank;
	cin >> rank;
	puzzle begin = begin_puzzle(rank);
	astar(begin);
	return 0;
}
