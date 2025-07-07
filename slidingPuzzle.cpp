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

int N = 3;

vector<boost::unordered_map<size_t, puzzle>> closed(N);
vector<boost::unordered_map<size_t, puzzle>> open(N);
vector<priority_queue<pair<long long, puzzle>>> pq(N);
vector<queue<puzzle>> q(N);
bool solved = false;

vector<mutex> q_mtx(N);
mutex solved_mtx;

void closed_insert(puzzle p, int id){
    closed[id][fhash(p)] = p;
}

bool closed_find(puzzle p, int id){
    bool ans;
    if(closed[id].find(fhash(p)) == closed[id].end())
        ans = false;
    else
        ans = true;
    return ans;
}

void open_insert(puzzle p, int id){
    open[id][fhash(p)] = p;
}

void open_erase(puzzle p, int id){
    open[id].erase(fhash(p));
}

bool open_find(puzzle p, int id){
    bool ans;
    if(open[id].find(fhash(p)) == open[id].end())
        ans = false;
    else
        ans = true;
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

void backtrack(puzzle p, int id){
	if(p.p_coord == 0){
		print_puzzle(p);
		return;
	}
	puzzle parent = closed[id][p.p_coord];
	backtrack(parent, id);
	print_puzzle(p);
}

size_t hash_thread(puzzle p){
    return (fhash(p) % N);
}

void enqueue(puzzle p, size_t id){
    q_mtx[id].lock();
    q[id].emplace(p);
    q_mtx[id].unlock();
}

queue<puzzle> copy_queue(int id){
    q_mtx[id].lock();
    queue<puzzle> cpy;
    while(!q[id].empty()){
        puzzle p = q[id].front();
        q[id].pop();
        cpy.push(p);
    }
    q_mtx[id].unlock();
    return cpy;
}

void pq_push(puzzle p, int id){
    pq[id].push({-(weight(p) + p.steps), p});
}

void dequeue(int id){
    queue<puzzle> local = copy_queue(id);
    while(!local.empty()){
        puzzle p = local.front();
        local.pop();
        if(!closed_find(p, id) && !open_find(p, id)){
            pq_push(p, id);
            open_insert(p, id);
        }
    }
}

puzzle pq_top(int id){
    while(pq[id].empty())
        dequeue(id);
    puzzle p = pq[id].top().second;
    pq[id].pop();
    return p;
}


void astar_thread(int id){
    cout << "Started thread " << id << "\n";
    while(true){
        if(solved)
            return;
		puzzle current = pq_top(id);
		closed_insert(current, id);
        open_erase(current, id);
		vector<puzzle> adj = get_adj(current);
		
		for(int i = 0; i < adj.size(); i++){
			if(weight(adj[i]) == 0){
                if(solve_puzzle()){
                    backtrack(adj[i], id);
                }
				return;
			}
		}

		for(int i = 0; i < adj.size(); i++){
            size_t t_index = hash_thread(adj[i]);
			if(t_index == id && !closed_find(adj[i], id) && !open_find(adj[i], id)){
                pq_push(adj[i], id);
                open_insert(adj[i], id);
            }
            else
                enqueue(adj[i], t_index);
		}
        dequeue(id);
	}
}

void astar(puzzle begin){
	cout << "begin astar\n";
    for(int i = 0; i < N; i++){
    	open[i][fhash(begin)] = begin;
	    pq[i].push({- (weight(begin) + begin.steps), begin});
    }
    vector<thread> ts;
    for(int i = 0; i < N; i++)
        ts.emplace_back(astar_thread, i);
    
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
	astar(begin);
	return 0;
}
// Função de hash Coord.h:67

// OpenList com hash map: PriorityList.:30
//            hashed_unique<

//tag<pos>,  BOOST_MULTI_INDEX_MEMBER(Node<N>,Coord<N>,pos)>,

 