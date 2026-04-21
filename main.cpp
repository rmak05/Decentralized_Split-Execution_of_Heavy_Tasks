#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <random>
#include <chrono>
#include <cassert>
#include <unistd.h>
using namespace std;

typedef vector<int> vi;
typedef pair<int, int> pi;
#define pb push_back
#define x first
#define y second

mt19937 rand_gen((unsigned)chrono::steady_clock::now().time_since_epoch().count());

// Use - `g++ main.cpp && a.exe --live < sample_input.txt` to compile in cmd

void error(const string &err, int exit_code, bool condition){
	if(condition){
        cout << "\n\nERROR: " << err << endl;
        exit(exit_code);
    }
}

const int FRAME_DELAY = 50 * 1e3;     // us
bool LIVE = false;

enum State{
    COMPUTATION,
    SIGNAL
};

int rand_num(int l=1, int r=1e9){
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<long long> dis(l, r);
    return dis(gen);
}

struct Message{
    int id;                     // Unique id for each message
    int x, y;                   // Origin location of message
    int target;                 // Destination component for Message. (-1 => Return to Origin)

    Message(pi origin, int target){
        id = rand_num();
        x = origin.x;
        y = origin.y;
        this->target = target;
    }
};

int distance(pi a, pi b){
    return abs(a.x - b.x) + abs(a.y - b.y); // Manhattan distance
}

struct Component{
    inline static int R;            // Communication radius for each component
    inline static int k;            // Total number of unique Components
    int id;                         // Component number 
    pi loc;                         // Location of Component (part of D[])
    int service_rate;               // Given.
    vector<Component*> neighbors;   // Neighboring Components to given component within R. 
    queue<Message> q;               // Queue of Tasks.
    set<int> seen_messages;         // Set of seen_messages (visited array to prevent infinite flooding)

    static void staticVar(int k1, int R1){
        k = k1;
        R = R1;
    }

    void addEvent(Message &m){
        if(seen_messages.find(m.id) != seen_messages.end()) return;
        q.push(m);
        seen_messages.insert(m.id);
    }

    void addEvent(pi origin, int target = 0){
        Message m(origin, target);
        q.push(m);
        seen_messages.insert(m.id);
    }

    void processEvent(){
        if(q.empty()) return;
        Message m = q.front();
        if(m.target != id) return;
        q.pop();

        Message processed_msg({m.x, m.y}, m.target + 1);
        if(processed_msg.target == k) processed_msg.target =- 1;
        q.push(processed_msg);
    }

    void forward(){
        if(q.empty()) return;
        Message m = q.front();
        if(m.target == id) return;
        q.pop();

        if(m.target == -1 && (distance(loc, {m.x, m.y}) <= R)){
            // Task Finished and Reached Destination
        }
        else{
            for(auto *c : neighbors) c->addEvent(m);
        }
    }
};

struct TaskArrivalDistribution{
private:
    poisson_distribution<int> dist;

public:
    TaskArrivalDistribution() :
    dist(0){}

    TaskArrivalDistribution(const vector<int>& _params){
        assert(_params.size() >= 1);

        dist = poisson_distribution<int>(_params[0]);
    }

    void set_params(const vector<int>& _params){
        assert(_params.size() >= 1);

        dist = poisson_distribution<int>(_params[0]);
    }

    int generate_value(){
        return dist(rand_gen);
    }
};

vector<vi> network(int R, vector<pi> &D){    // O(n*n)
    int n = D.size();
    vector<vi> mesh(n);

    for(int i = 0; i < n - 1; i++){
        for(int j = i + 1; j < n; j++){
            if(distance(D[i], D[j]) <= R){
                mesh[i].pb(j);
                mesh[j].pb(i);
            }
        }
    }

    return mesh;
}

vector<Component> distribute_components(vector<pi> &D, vi &Cmap, vi &service){
    int n = D.size(), k = service.size();
    vector<Component> C(n);
    for(int i = 0; i < n; i++){
        C[i].loc = D[i];    /* NOTE :- C[i].loc and D[i] Should always be the same. So that mesh created from initial D[] remains valid for finally generated components C[]. Removes recomputation. */

        C[i].id = Cmap[i];
        // C[i].id = i % k;      // Assign Components to Devices. Only this can be changed.

        C[i].service_rate = service[C[i].id];
    }
    return C;
}

vector<vector<vi>> find_C(int A, int R, vector<Component> &C){      // O(n*R*R)
    vector<vector<vi>> first_hop(A, vector<vi>(A));

    int n = C.size();
    for(int u = 0; u < n; u++){
        auto [x, y] = C[u].loc;
        for(int dx = -R; dx <= R; dx++){
            int DY = R - abs(dx);
            for(int dy = -DY; dy <= DY; dy++){
                int i = x + dx, j = y + dy;
                if(i < 0 || j < 0 || i >= A || j >= A) continue;
                first_hop[i][j].pb(u);
            }
        }
    }

    return first_hop;
}

vector<pair<int, int>> EventArrival(vector<vector<TaskArrivalDistribution>>& arrival_dist){
    int rows = arrival_dist.size(), cols = arrival_dist[0].size();
    vector<pair<int, int>> events;
    
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            int num_events = arrival_dist[i][j].generate_value();

            for(int k = 0; k < num_events; k++){
                events.emplace_back(i, j);
            }
        }
    }

    return events;
}

int Print(vector<Component> &C, State state){
    int n = C.size();
    cout << "\nCurrent State - " << ((state == SIGNAL) ? "SIGNAL" : "COMPUTATION") << "\n";
    for(int i = 0; i < n; i++){
        printf("   Device %d:  (%d,%d)  Component %d    -   Queue length = %d\n", i, C[i].loc.x, C[i].loc.y, C[i].id, (int)C[i].q.size());
    }
    int lines = n + 2;
    fflush(stdout);
    return lines;
}

void Refresh(int lines){
    if(lines==0) return;
    // Clear and print on same lines
    printf("\033[%dA", lines);
    for(int i = 0; i < lines; i++) printf("\033[2K\n");
    printf("\033[%dA", lines);
}

pair<float, float> start_simulation(int R, vector<pi> &D, vi &Cmap, vi &service, vector<vi> &arrival, int computation_period, int signal_period, int simulation_time){
    int A = arrival.size(), n = D.size(), k = service.size();
    
    int grid_rows = arrival.size();
    int grid_cols = arrival[0].size();
    vector<vector<TaskArrivalDistribution>> arrival_dist(grid_rows, vector<TaskArrivalDistribution>(grid_cols));
    for(int i = 0; i < grid_rows; i++){
        for(int j = 0; j < grid_cols; j++){
            arrival_dist[i][j].set_params({arrival[i][j]});
        }
    }

    auto mesh = network(R, D);
    Component::staticVar(k, R);
    auto C = distribute_components(D, Cmap, service);
    for(int u = 0; u < n; u++){
        for(int v : mesh[u]){
            C[u].neighbors.pb(&C[v]);
        }
    }
    auto first_hop = find_C(A, R, C);       // All Reachable Components from each Cell.

    const int HOPS_PER_SIGNAL_PERIOD = 2;
    State state = SIGNAL;                   // Currently in signal or computation
    int Time = 0;
    long long totalQueueLength = 0, totalFrames = 0;
    long long TaskCompletionTime = 0, totalTasks = 0;
    long long totalQueueTime = 0, totalCommunicationTime = 0;
    bool PRINTED = false;
    
    while(true){
        int lines = Print(C, state);

        int pending_tasks = 0;
        for(Component &c : C) pending_tasks += c.q.size();

        if(state == COMPUTATION){
            for(Component &c : C){
                if((Time % (1000/c.service_rate)) == 0) c.processEvent();
            }

            if(Time >= computation_period){
                Time = 0;
                state = SIGNAL;
            }
        }
        else if(state == SIGNAL){
            for(Component &c : C){
                if((Time % (1000/c.service_rate)) == 0) c.forward();
            }

            if(Time >= signal_period){
                Time = 0;
                state = COMPUTATION;

                if(totalFrames*FRAME_DELAY <= simulation_time*1e6){
                    auto event_coords = EventArrival(arrival_dist);
                    
                    for(auto [x, y] : event_coords){
                        for(int u : first_hop[x][y]){
                            C[u].addEvent({x, y});
                        }
                        totalTasks++;
                    }
                }
                else{
                    if(!PRINTED){
                        Refresh(lines);
                        cout << "\nNew Event Generation stopped." << endl;
                        lines = 0;
                        PRINTED = true;
                    }

                    if(pending_tasks == 0) break;
                }
            }
        }

        totalQueueLength += pending_tasks;
        Time += FRAME_DELAY / 1000;
        totalFrames++;
        if(LIVE) usleep(FRAME_DELAY);

        Refresh(lines);
    }

    float L = (float)totalQueueLength / totalFrames;
    float T = L * FRAME_DELAY / (totalFrames * totalTasks);

    return {T, L};
}

tuple<int, vector<pi>, vi, vector<vi>, int, int, int> Input(){
    int A, R, n, k, computation_period, signal_period, simulation_time;

    cout << "\nEnter Square length A: ";
    cin >> A;
    cout << "\nEnter communication radius R: ";
    cin >> R;
    cout << "\nEnter the number of devices D:\n";
    cin >> n;
    cout << "\nEnter the number of components C: ";
    cin >> k;
    error("|D| should be a multiple of |C|", 1, n % k);

    cout << "\nEnter Component computation_period (in ms): ";
    cin >> computation_period;
    cout << "\nEnter Component signal_period (in ms): ";
    cin >> signal_period;
    cout << "\nEnter Time to run a single simulation (in seconds): ";
    cin >> simulation_time;

    vector<pi> D(n);
    vi service(k);
    vector<vi> arrival(A, vi(A));

    cout << "\nEnter locations of devices (0-based) D[]: ";
    for(int i = 0; i < n; i++) cin >> D[i].x >> D[i].y;

    cout << "\nEnter the service rate of each component C_i (tasks per second): ";
    for(int i = 0; i < k; i++) cin >> service[i];

    cout << "\nEnter the Task arrival Rate at each cell:" << endl;
    for(int i = 0; i < A; i++){
        for(int j = 0; j < A; j++){
            cin >> arrival[i][j];
        }
    }

    return {R, D, service, arrival, computation_period, signal_period, simulation_time};
}

vi generateC(int n, int k){
    vi Cmap(n);
    for(int i=0;i<n;i++) Cmap[i] = i%k;
    for(int i=0;i<n-1;i++){
        int j = rand_num(i,n-1);
        swap(Cmap[i], Cmap[j]);
    }
    return Cmap;
}

int main(int argc, char* argv[]){
    srand(time(NULL));

    if(argc > 1){
        if(strcmp(argv[1], "--live") == 0) LIVE = true;
    }

    auto [R, D, service, arrival, cp, sp, st] = Input();
    Refresh(11);
    
    const int samples = 3, width = 60;
    for(int i=1;i<=samples;i++){
        for(int i=0;i<width;i++) cout<<"-";
        cout<<"\nSample : "<<i<<endl;

        vi Cmap = generateC(D.size(), service.size());
        auto [T, L] = start_simulation(R, D, Cmap, service, arrival, cp, sp, st);

        cout << "\nAverage Time per packet T  = " << T << " seconds" << endl;
        cout << "Average Queue Length    L  = " << L << endl;

        for(int i=0;i<width;i++) cout<<"="; cout<<endl;

        sleep(1);
    }

    return 0;
}