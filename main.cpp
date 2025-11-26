#include <iostream>
#include <queue>
#include <vector>
#include <fstream>
#include <memory>
#include <algorithm>
#include <cmath>
#include <list>
#include <climits>
using namespace std;

enum class OrderType { NORMAL, VEGAN, VIP };
enum class ChefType { NORMAL, VEGAN, VIP };

struct Order {
	int id;
	OrderType type;
	int RT;       // arrival time
	int size;     // units of work
	double money; // used for VIP priority

	// runtime
	int ST = -1;    // start time
	int FT = -1;    // finish time
	int WT = -1;    // waiting time
	int STime = -1; // service time
	bool autoPromoted = false;

	Order(int _id, OrderType _t, int _rt, int _size, double _m)
		: id(_id), type(_t), RT(_rt), size(_size), money(_m) {}
};

using OrderPtr = shared_ptr<Order>;

struct Chef {
	int id;
	ChefType type;
	double speed; // units per time unit
	int nextFree{0}; // time when chef becomes free
	int servedCount{0};

	Chef(int _id, ChefType t, double s) : id(_id), type(t), speed(s) {}
};

using ChefPtr = shared_ptr<Chef>;

// Comparator for VIP priority queue: higher money first, then earlier RT
struct VipComp {
	bool operator()(const OrderPtr &a, const OrderPtr &b) const {
		if (a->money != b->money) return a->money < b->money; // max-heap by money
		return a->RT > b->RT;
	}
};

int main() {
	// Simple, improved single-file restaurant simulation.
	// Input format (same as the original program):
	// Line1: N G V
	// Line2: SN SG SV   (speeds for Normal, Vegan, VIP chefs)
	// Line3: M  (number of orders)
	// Next M lines: <TYPE_CHAR> <RT> <ID> <SIZE> <MONEY>
	// TYPE_CHAR: 'G' = Vegan, 'V' = VIP, otherwise Normal

	ifstream fin("input.txt");
	if(!fin) {
		cerr << "Failed to open input.txt\n";
		return 1;
	}

	int N, G, V;
	fin >> N >> G >> V;
	double SN, SG, SV;
	fin >> SN >> SG >> SV;
	int M; fin >> M;

	vector<OrderPtr> arrivals;
	arrivals.reserve(M);
	for(int i=0;i<M;++i){
		char typ; int RT, ID, SIZE; double MON;
		fin >> typ >> RT >> ID >> SIZE >> MON;
		OrderType t = OrderType::NORMAL;
		if(typ=='G') t = OrderType::VEGAN;
		else if(typ=='V') t = OrderType::VIP;
		arrivals.push_back(make_shared<Order>(ID,t,RT,SIZE,MON));
	}

	// sort arrivals by RT just in case
	sort(arrivals.begin(), arrivals.end(), [](const OrderPtr &a, const OrderPtr &b){
		if(a->RT!=b->RT) return a->RT < b->RT;
		return a->id < b->id;
	});
		
	// create chefs
	vector<ChefPtr> chefs;
	int cid = 1;
	for(int i=0;i<N;++i) chefs.push_back(make_shared<Chef>(cid++, ChefType::NORMAL, SN));
	for(int i=0;i<G;++i) chefs.push_back(make_shared<Chef>(cid++, ChefType::VEGAN, SG));
	for(int i=0;i<V;++i) chefs.push_back(make_shared<Chef>(cid++, ChefType::VIP, SV));

	// queues
	priority_queue<OrderPtr, vector<OrderPtr>, VipComp> vipQ;
	queue<OrderPtr> normalQ;
	queue<OrderPtr> veganQ;

	list<OrderPtr> finished; // store finished orders

	// simulation state
	int time = 0;
	size_t nextArrival = 0;
	const int AUTO_PROMOTE_THRESHOLD = 10; // time units
	int autoPromotedCount = 0;

	// main loop: run until all arrivals processed, queues empty, and all chefs free
	while(true) {
		// move arrivals at current time into queues
		while(nextArrival < arrivals.size() && arrivals[nextArrival]->RT <= time) {
			auto o = arrivals[nextArrival++];
			if(o->type == OrderType::VIP) vipQ.push(o);
			else if(o->type == OrderType::VEGAN) veganQ.push(o);
			else normalQ.push(o);
		}

		// auto-promote normals waiting longer than threshold
		int normalSize = (int)normalQ.size();
		for(int i=0;i<normalSize;++i){
			auto o = normalQ.front(); normalQ.pop();
			if(time - o->RT >= AUTO_PROMOTE_THRESHOLD){
				o->type = OrderType::VIP;
				o->autoPromoted = true;
				vipQ.push(o);
				++autoPromotedCount;
			} else normalQ.push(o);
		}

		// try to assign orders to free chefs at this time
		for(auto &chef : chefs){
			if(chef->nextFree > time) continue; // busy

			// choose order for this chef
			OrderPtr chosen = nullptr;
			if(!vipQ.empty()){
				chosen = vipQ.top(); vipQ.pop();
			} else {
				if(chef->type == ChefType::VEGAN){
					if(!veganQ.empty()){ chosen = veganQ.front(); veganQ.pop(); }
					else if(!normalQ.empty()){ chosen = normalQ.front(); normalQ.pop(); }
				} else if(chef->type == ChefType::NORMAL){
					if(!normalQ.empty()){ chosen = normalQ.front(); normalQ.pop(); }
					else if(!veganQ.empty()){ chosen = veganQ.front(); veganQ.pop(); }
				} else { // VIP chef
					if(!vipQ.empty()){ chosen = vipQ.top(); vipQ.pop(); }
					else if(!normalQ.empty()){ chosen = normalQ.front(); normalQ.pop(); }
					else if(!veganQ.empty()){ chosen = veganQ.front(); veganQ.pop(); }
				}
			}

			if(chosen){
				chosen->ST = time;
				chosen->STime = max(1, (int)ceil(chosen->size / chef->speed));
				
				chosen->WT = chosen->ST - chosen->RT;
				chosen->FT = time + chosen->STime;

				chef->nextFree = chosen->FT;
				chef->servedCount++;

				// push to finished list (we keep record now)
				finished.push_back(chosen);
			}
		}

		// check termination condition
		bool anyBusy = false;
		int nextChefFree = INT_MAX;
		for(auto &c : chefs){
			if(c->nextFree > time) { anyBusy = true; nextChefFree = min(nextChefFree, c->nextFree); }
		}

		bool queuesEmpty = vipQ.empty() && normalQ.empty() && veganQ.empty();
		bool arrivalsLeft = nextArrival < arrivals.size();

		if(!arrivalsLeft && queuesEmpty && !anyBusy) break;

		// advance time to next interesting event: next arrival or next chef free
		int nextTime = INT_MAX;
		if(arrivalsLeft) nextTime = min(nextTime, arrivals[nextArrival]->RT);
		if(anyBusy) nextTime = min(nextTime, nextChefFree);
		if(nextTime==INT_MAX) break; // nothing else
		// if nextTime is same as current (possible), increment by 1 to avoid tight loop
		if(nextTime <= time) ++time; else time = nextTime;
	}

	// prepare output: sort finished by FT
	vector<OrderPtr> done(finished.begin(), finished.end());
	sort(done.begin(), done.end(), [](const OrderPtr &a, const OrderPtr &b){
		if(a->FT != b->FT) return a->FT < b->FT;
		return a->id < b->id;
	});

	// write output file
	ofstream ofs("output.txt");
	ofs << "FT ID RT WT ST\n";
	double totalWT=0, totalST=0; int cnt=0;
	int cntN=0,cntV=0,cntVIP=0;
	for(auto &o : done){
		ofs << o->FT << ' ' << o->id << ' ' << o->RT << ' ' << o->WT << ' ' << o->STime << '\n';
		totalWT += o->WT; totalST += o->STime; ++cnt;
		if(o->type==OrderType::NORMAL) ++cntN;
		else if(o->type==OrderType::VEGAN) ++cntV;
		else ++cntVIP;
	}

	ofs << "\nSummary:\n";
	ofs << "Orders: "<<cnt<<" (Normal="<<cntN<<" Vegan="<<cntV<<" VIP="<<cntVIP<<")\n";
	ofs << "Chefs: ";
	int nN=0,nVg=0,nVp=0;
	for(auto &c:chefs){ if(c->type==ChefType::NORMAL) ++nN; else if(c->type==ChefType::VEGAN) ++nVg; else ++nVp; }
	ofs << "Normal="<<nN<<" Vegan="<<nVg<<" VIP="<<nVp<<"\n";
	ofs << "Average Waiting Time: " << (cnt? totalWT/cnt : 0) << "\n";
	ofs << "Average Service Time: " << (cnt? totalST/cnt : 0) << "\n";
	ofs << "% Auto-promoted: " << (done.size()? (100.0*autoPromotedCount/done.size()) : 0) << "\n";

	cout << "Simulation completed. Output written to output.txt\n";
	return 0;
}

