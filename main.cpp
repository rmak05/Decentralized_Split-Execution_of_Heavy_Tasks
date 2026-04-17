#include <iostream>
#include <vector>
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

vector<Component> distribute_components(int R, vector<pi> &D, vi &service, vector<vi> &arrival){
    vector<Component> C(D.size());
    return C;
}

vector<vi> network(int R, vector<Component> &C){    // O(n*n)
    int n = C.size();
    vector<vi> mesh(n);

    for(int i=0;i<n-1;i++){
        for(int j=i+1;j<n;j++){
            if((C[j].id == (C[i].id+1)) && (distance(C[i].loc,C[j].loc)<=R)){
                mesh[i].pb(j);
            }
        }
    }

    return mesh;
}

vector<vector<vi>> find_nearest_C1(int A, int R, vector<Component> &C){
    vector<vector<vi>> nearest_C1(A,vector<vi>(A));

    return nearest_C1;
}

void start_simulation(int R, vector<pi> &D, vi &service, vector<vi> &arrival){
    int A = arrival.size(), n = D.size(), k = service.size();

    vector<Component> C = distribute_components(R, D, service, arrival);
    vector<vi> mesh = network(R, C);                 // All reachable C_i --> C_(i+1) Components
    vector<vector<vi>> nearest_C1 = find_nearest_C1(A, R, C);  // Nearest C1 Component from each Cell 

    while(true){


        usleep(FRAME_DELAY);
    }
}

tuple<int, vector<pi>, vi, vector<vi>> Input(){
    int A, R, n, k;
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

    cout<<"Enter locations of devices (0-based) D[]: ";
    for(int i=0;i<n;i++) cin>>D[i].x>>D[i].y;

    cout<<"Enter the service rate of each component C_i: ";
    for(int i=0;i<k;i++) cin>>service[i];

    cout<<"Enter the Task arrival Rate at each cell: "<<endl;
    for(int i=0;i<A;i++) for(int j=0;j<A;j++) cin>>arrival[i][j];
    
    return {R, D, service, arrival};
}

int main(){
    ios_base::sync_with_stdio(false);
    cin.tie(0);


    auto [R, D, service, arrival] = Input();
    start_simulation(R, D, service, arrival);

    return 0;
}