#include <iostream>
#include <vector>
#include <format>
#include <queue>
#include <unistd.h>
using namespace std;
typedef vector<int> vi;
typedef pair<int,int> pi;
#define pb push_back
#define x first
#define y second

void error(const string &err, int exit_code, bool condition){
	if(condition){
		cout<<"\n\nERROR: "<<err<<endl;
		exit(exit_code);
	}
}

const int FRAME_DELAY = 5000;     // us

struct Component{
    int id;             // Component number 
    pi loc;             // Location of Component (part of D[])
    int service_rate;   // Given.
};

int distance(pi a, pi b){
    return abs(a.x-b.x) + abs(a.y-b.y);     // Manhattan distance
}

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

void start_simulation(int R, vector<pi> &D, vi &service, vector<vi> &arrival, int computation_period, int siganl_period){
    int A = arrival.size(), n = D.size(), k = service.size();

    auto mesh = network(R, D);
    auto C = distribute_components(mesh, D, service, arrival);
    auto first_hop = find_C(A, R, C);       // All Reachable Components from each Cell.

    while(true){


        usleep(FRAME_DELAY);
    }
}

tuple<int, vector<pi>, vi, vector<vi>, int, int> Input(){
    int A, R, n, k, computation_period, signal_period;
    vector<pi> D(n);
    vi service(k);
    vector<vi> arrival(A,vi(A));

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

    cout<<"Enter locations of devices (0-based) D[]: ";
    for(int i=0;i<n;i++) cin>>D[i].x>>D[i].y;

    cout<<"Enter the service rate of each component C_i: ";
    for(int i=0;i<k;i++) cin>>service[i];

    cout<<"Enter the Task arrival Rate at each cell: "<<endl;
    for(int i=0;i<A;i++) for(int j=0;j<A;j++) cin>>arrival[i][j];
    
    return {R, D, service, arrival, computation_period, signal_period};
}

int main(){
    ios_base::sync_with_stdio(false);
    cin.tie(0);

    auto [R, D, service, arrival, cp, sp] = Input();
    
    start_simulation(R, D, service, arrival, cp, sp);

    return 0;
}