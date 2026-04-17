#include <iostream>
#include <random>
#include <format>
#include <vector>
#include <queue>
#include <set>
#include <unistd.h>
using namespace std;
typedef vector<int> vi;
typedef pair<int,int> pi;
#define pb push_back
#define x first
#define y second

// Use - `g++ -std=c++20 main.cpp` and `./a.exe --live` to compile

void error(const string &err, int exit_code, bool condition){
	if(condition){
		cout<<"\n\nERROR: "<<err<<endl;
		exit(exit_code);
	}
}

const int FRAME_DELAY = 5000;     // us
bool LIVE = false;

enum State{
    COMPUTATION,
    SIGNAL
};

int rand_num(){
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<long long> dis(1, 1e9);
    return dis(gen);
}

struct Message{
    int id;                     // Unique id for each message
    int x,y;                    // Origin location of message
    int target;                 // Destination component for Message. (-1 => Return to Origin)

    Message(pi origin, int target){
        id = rand_num();
        x = origin.x;
        y = origin.y;
        this->target = target;
    }
};

int distance(pi a, pi b){
    return abs(a.x-b.x) + abs(a.y-b.y);     // Manhattan distance
}

struct Component{
    inline static int R;        // Communication radius for each component
    inline static int k;        // Total number of unique Components
    int id;                     // Component number 
    pi loc;                     // Location of Component (part of D[])
    int service_rate;           // Given.
    vector<Component> neighbors;// Neighboring Components to given component within R. 
    queue<Message> q;           // Queue of Tasks.
    set<int> seen_messages;     // Set of seen_messages (visited array to prevent infinite flooding)

    static void staticVar(int k1, int R1){
        k = k1;
        R = R1;
    }

    void addEvent(Message &m){
        if(seen_messages.find(m.id) != seen_messages.end()) return;
        q.push(m);
        seen_messages.insert(m.id);
    }

    void addEvent(pi origin, int target=0){
        Message m(origin, target);
        q.push(m);
        seen_messages.insert(m.id);
    }

    void processEvent(){
        if(q.empty()) return;
        Message m = q.front();
        if(m.target!=id) return;
        q.pop();

        Message processed_msg({m.x,m.y}, m.id+1);
        if(m.id==k) m.id=-1;
        q.push(processed_msg);
    }

    void forward(){
        if(q.empty()) return;
        Message m = q.front();
        if(m.target==id) return;
        q.pop();

        if(m.target==-1 && distance(loc, {m.x,m.y})){
            // Task Finished and Reached Destination
        }
        else{
            for(Component &c:neighbors) addEvent(m);
        }
    }
};

vector<vi> network(int R, vector<pi> &D){    // O(n*n)
    int n = D.size();
    vector<vi> mesh(n);

    for(int i=0;i<n-1;i++){
        for(int j=i+1;j<n;j++){
            if(distance(D[i], D[j])<=R){
                mesh[i].pb(j);
                mesh[j].pb(i);
            }
        }
    }

    return mesh;
}

vector<Component> distribute_components(vector<vi> &mesh, vector<pi> &D, vi &service, vector<vi> &arrival){
    int n = D.size(), k = service.size();
    vector<Component> C(n);
    for(int i=0;i<n;i++){
        C[i].loc = D[i];    /* NOTE :- C[i].loc and D[i] Should always be the same. So that mesh created from initial D[] remains valid for finally generated components C[]. Removes recomputation. */

        C[i].id = i%k;      // Assign Components to Devices. Only this can be changed.

        C[i].service_rate = service[C[i].id];
        for(int v:mesh[i]) C[i].neighbors.pb(C[v]);
    }
    return C;
}

vector<vector<vi>> find_C(int A, int R, vector<Component> &C){      // O(n*R*R)
    vector<vector<vi>> first_hop(A,vector<vi>(A));

    int n = C.size();
    for(int u=0;u<n;u++){
        auto [x,y] = C[u].loc;
        for(int dx=-R;dx<=R;dx++){
            int DY = R-abs(dx);
            for(int dy=-DY;dy<=DY;dy++){
                int i=x+dx, j=y+dy;
                if(i<0 || j<0 || i>=A || j>=A) continue;
                if(dx==0 && dy==0){
                    error(format("Multiple Components/Devices {} and {} at same cell ({},{})",u,-first_hop[i][j][0]-1,i,j), 2, first_hop[i][j].size());
                    first_hop[i][j].pb(-u-1);
                }
                else first_hop[i][j].pb(u);
            }
        }
    }

    return first_hop;
}

pi EventArrival(vector<vi> &arrival){
    return {-1, -1};
}

tuple<float, float, float, float> start_simulation(int R, vector<pi> &D, vi &service, vector<vi> &arrival, int computation_period, int signal_period){
    int A = arrival.size(), n = D.size(), k = service.size();

    auto mesh = network(R, D);
    Component::staticVar(k, R);
    auto C = distribute_components(mesh, D, service, arrival);
    auto first_hop = find_C(A, R, C);       // All Reachable Components from each Cell.

    const int HOPS_PER_SIGNAL_PERIOD = 2;
    State state = SIGNAL;                   // Currently in signal or computation
    int Time = 0;
    long long totalQueueLength = 0, totalFrames = 0;
    long long TaskCompletionTime = 0, totalTasks = 0;
    long long totalQueueTime = 0, totalCommunicationTime = 0;

    while(true){
        if(state==COMPUTATION){
            if(Time>=computation_period){
                for(Component &c:C) c.processEvent();
                Time = 0;
                state = SIGNAL;
            }
        }
        else if(state==SIGNAL){
            if((Time%(signal_period/HOPS_PER_SIGNAL_PERIOD))==0){
                for(Component &c:C) c.forward();
            }

            if(Time>=signal_period){
                Time = 0;
                state = COMPUTATION;
            }
        }

        auto [x,y] = EventArrival(arrival);
        if(x!=-1){      // Process new incoming event
            for(int u:first_hop[x][y]) C[u].addEvent({x,y});
            totalTasks++;
        }

        for(Component &c:C) totalQueueLength += c.q.size();
        Time += FRAME_DELAY;
        totalFrames++;
        if(LIVE) usleep(FRAME_DELAY);
    }

    float L = (float)totalQueueLength/totalFrames;
    float T = L*FRAME_DELAY/(totalFrames*totalTasks);
    
    /* NOT DONE. Fraction of total delay caused due to queueing at components and communication between components. */
    /* Service Rate - NOT USED anywhere in the code. */

    return {T,L,1,0};
}

tuple<int, vector<pi>, vi, vector<vi>, int, int> Input(){
    int A, R, n, k, computation_period, signal_period;

    cout<<"Enter Square length A: ";
    cin>>A;
    cout<<"Enter communication radius R: ";
    cin>>R;
    cout<<"Enter the number of devices D: ";
    cin>>n;
    cout<<"Enter the number of components C: ";
    cin>>k;
    error("|D| should be a multiple of |C|", 1, n%k);

    cout<<"Enter Component computation_period: ";
    cin>>computation_period;
    cout<<"Enter Component signal_period: ";
    cin>>signal_period;
    
    vector<pi> D(n);
    vi service(k);
    vector<vi> arrival(A,vi(A));

    cout<<"Enter locations of devices (0-based) D[]: ";
    for(int i=0;i<n;i++) cin>>D[i].x>>D[i].y;

    cout<<"Enter the service rate of each component C_i: ";
    for(int i=0;i<k;i++) cin>>service[i];

    cout<<"Enter the Task arrival Rate at each cell: "<<endl;
    for(int i=0;i<A;i++) for(int j=0;j<A;j++) cin>>arrival[i][j];
    
    return {R, D, service, arrival, computation_period, signal_period};
}

int main(int argc, char* argv[]){
    if(argc>1){
        if(strcmp(argv[1],"--live")) LIVE = true;
    }

    auto [R, D, service, arrival, cp, sp] = Input();
    
    auto [T, L, fraction_queueing_delay, fraction_communication_delay] = start_simulation(R, D, service, arrival, cp, sp);

    return 0;
}