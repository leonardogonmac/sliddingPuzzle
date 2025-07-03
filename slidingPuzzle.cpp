#include <bits/stdc++.h>
#include <sys/resource.h>
#include "boost/multi_array.hpp"
#include <boost/multi_index/hashed_index.hpp>
#include <boost/unordered_map.hpp>


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
	size_t p_coord;
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

size_t fhash(puzzle p){
    size_t seed = 0;
    for(int i = 0; i < p.m.size(); i++){
        for(int j = 0; j < p.m[i].size(); j++){
            boost::hash_combine(seed, p.m[i][j].num);
        }
    }
    return seed;
}


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
    new_p.p_coord = fhash(p);
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

boost::unordered_map<size_t, puzzle> closed;
boost::unordered_map<size_t, puzzle> open;
priority_queue<pair<long long, puzzle>> pq;
bool solved = false;
mutex solved_mtx, closed_mtx, open_mtx, pq_mtx;

puzzle pq_top(){
    while(pq_mtx.lock(), pq.empty())
        pq_mtx.unlock();
    puzzle p = pq.top().second;
    pq.pop();
    pq_mtx.unlock();
    return p;
}

void pq_push(puzzle p){
    pq_mtx.lock();
    pq.push({-(weight(p) + p.steps), p});
    pq_mtx.unlock();
}

void closed_insert(puzzle p){
    closed_mtx.lock();
    closed[fhash(p)] = p;
    closed_mtx.unlock();
}

bool closed_find(puzzle p){
    bool ans;
    closed_mtx.lock();
    if(closed.find(fhash(p)) == closed.end())
        ans = false;
    else
        ans = true;
    closed_mtx.unlock();
    return ans;
}

void open_insert(puzzle p){
    open_mtx.lock();
    open[fhash(p)] = p;
    open_mtx.unlock();
}

void open_erase(puzzle p){
    open_mtx.lock();
    open.erase(fhash(p));
    open_mtx.unlock();
}

bool open_find(puzzle p){
    bool ans;
    open_mtx.lock();
    if(open.find(fhash(p)) == open.end())
        ans = false;
    else
        ans = true;
    open_mtx.unlock();
    return ans;
}


bool solve_puzzle(){
    solved_mtx.lock();
    if(solved){
        solved_mtx.unlock();
        return false;
    }
    else{
        solved = true;
        solved_mtx.unlock();
        return true;
    }
}

void backtrack(puzzle p){
	if(p.p_coord == 0){
		print_puzzle(p);
		return;
	}
	puzzle parent = closed[p.p_coord];
	backtrack(parent);
	print_puzzle(p);
}

void astar_thread(){
    cout << "Started thread\n";
    while(true){
        if(solved)
            return;
		puzzle current = pq_top();
		closed_insert(current);
		open_erase(current);
		vector<puzzle> adj = get_adj(current);
		
		for(int i = 0; i < adj.size(); i++){
			if(weight(adj[i]) == 0){
                if(solve_puzzle()){
                    backtrack(adj[i]);
                }
				return;
			}
		}

		for(int i = 0; i < adj.size(); i++){
			if(!closed_find(adj[i]) && !open_find(adj[i])){
				open_insert(adj[i]);
				pq_push(adj[i]);
			}
		}
	}
}

void astar(puzzle begin, int n){
	cout << "begin astar\n";
	open[fhash(begin)] = begin;
	pq.push({- (weight(begin) + begin.steps), begin});
    vector<thread> ts;
    for(int i = 0; i < n; i++)
        ts.emplace_back(astar_thread);
    
    for(thread& t: ts)
        t.join();
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
	begin.p_coord = 0;
	return begin;
}

// 1 2 3 4 5 6 ..

int main(){
	int rank;
	cin >> rank;
	puzzle begin = begin_puzzle(rank);
	astar(begin, 3);
	return 0;
}
// Função de hash Coord.h:67

// OpenList com hash map: PriorityList.:30
//            hashed_unique<

//tag<pos>,  BOOST_MULTI_INDEX_MEMBER(Node<N>,Coord<N>,pos)>,

 