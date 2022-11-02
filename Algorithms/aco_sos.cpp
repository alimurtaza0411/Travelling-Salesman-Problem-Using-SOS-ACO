#include <bits/stdc++.h>
#include <cstdio>
#include <iostream>
using namespace std;

// const float RHO = 0.1;
// const float QNOT = 0.99;
// const float BETA = 5.00;
// const float ALPHA = 1.00;
// const float GAMMA = 0.90;
// const float PHI = 0.1;
// const int ANT_COUNT = 100;
// const int MAX_ITERATIONS = 300;
// const float QVAL = 1000.00;
// const float MAX_PHEROMONE = 300.0;
// const float TAU_NOT = 1.00;

const float QMIN = 0.001;
const int N = 10;
int ITER_MAX = 100;
const int ANT_COUNT = 10;
const float RHO = 0.1;
const float Q = 1000.0;

struct City {
	float x;
	float y;
};

struct Edge {
	float distance;
	float pheromone;
};

vector<City> cities;
int numCities;

void parseData(string filename) {
    freopen(filename.c_str(), "r", stdin);
    int id;

    while(cin >> id) {
        float x, y;		
        City city;
        cin >> city.x >> city.y;
        cities.push_back(city);
    }   

    numCities = cities.size();
}

float euclideanDistance(City city1, City city2)
{
	return pow(pow(city1.x - city2.x, 2) + pow(city1.y - city2.y, 2), 0.5);
}

class Organism
{
public:
    float alpha;
    float beta;
    vector<vector<Edge>> edges;
    float fitnessValue;
	float bestDistance;
	vector<int> bestPath;

    Organism() {
        alpha = rand()/(float) RAND_MAX + rand()%4+1;
        beta = rand()/(float) RAND_MAX + rand()%4+1;

	    edges = vector<vector<Edge>> (numCities, vector<Edge> (numCities));
		for(int i=0;i<numCities;i++) {
			for(int j=i+1;j<numCities;j++) {
				edges[i][j].distance = euclideanDistance(cities[i],cities[j]);
	            edges[i][j].pheromone = 1.0;// edges[i][j].distance;
				edges[j][i] = edges[i][j];
			}
		}
		fitnessValue = 1/startACO(false);
		bestDistance = INT_MAX;
    }

    float startACO(bool performGU)
    {
        float optimalDistance = INT_MAX;
        vector<int> optimalPath;
        for (int ant = 0; ant < ANT_COUNT; ant++)
		{
			pair<float, vector<int>> path = tour();
			float antTourDistance = path.first;
			vector<int> antTourPath = path.second;
            
			if (antTourDistance < optimalDistance)
			{
				optimalDistance = antTourDistance;
				optimalPath = antTourPath;
			}
		}
		if(performGU && optimalDistance<bestDistance){
			bestDistance = optimalDistance;
			bestPath = optimalPath;
		}
        // Local Optimization (if necessary)
		if(performGU){
			// Perform Global Update
            globalUpdate(optimalPath, optimalDistance);
		}
		return optimalDistance;
    }

    pair<float, vector<int>> tour()
    {
        vector<bool> visited(numCities, false);
        vector<int> path;
        float distance = 0.0;

        int currentCity = rand() % numCities;
        int firstCity = currentCity;
        int visitedCitiesCount = 1;
        path.push_back(currentCity);
        visited[currentCity] = true;

        while (visitedCitiesCount < numCities)
        {
            int nextCity = getNextCity(visited, currentCity);
            path.push_back(nextCity);
            visitedCitiesCount++;
            visited[nextCity] = true;
            distance += edges[currentCity][nextCity].distance;
            currentCity = nextCity;
        }

        distance += edges[currentCity][firstCity].distance;
        path.push_back(firstCity);

        return {distance, path};
    }

    int getNextCity(vector<bool> &visited, int currentCity)
    {
        float q = float(rand() % 100) / 100;
        int nextCity;
        float maxProbability = INT_MIN;
		for (int allowedCity = 0; allowedCity < numCities; allowedCity++)
		{
			if (!visited[allowedCity])
			{
				float pheromone = edges[currentCity][allowedCity].pheromone;
				float distance = edges[currentCity][allowedCity].distance;
				float probability = pow(pheromone, alpha) * pow(1 / distance, beta);
				if (probability > maxProbability)
				{
					maxProbability = probability;
					nextCity = allowedCity;
				}
			}
		}
        return nextCity;
    }

    void globalUpdate(vector<int> path, float totalDistance)
    {
        // Evaporation
        for (int i = 0; i < numCities; i++)
        {
            for (int j = i + 1; j < numCities; j++)
            {
                float newPheromone = edges[i][j].pheromone * (1 - RHO);
                if (newPheromone < QMIN)
                    newPheromone = QMIN;
                edges[i][j].pheromone = edges[j][i].pheromone = newPheromone;
            }
        }

        // Deposit
        for (int i = 1; i < path.size(); i++)
        {
            int currentCity = path[i - 1];
            int nextCity = path[i];
            int distance = edges[currentCity][nextCity].distance;
            edges[currentCity][nextCity].pheromone += (Q / totalDistance); // * distance;
            edges[nextCity][currentCity].pheromone += (Q / totalDistance); // * distance;
        }
    }
};

class HybridSOSACO
{
    public:
    vector<Organism> organisms;
    int bestOrgIdx;

    HybridSOSACO()
    {
        organisms = vector<Organism> (N);
        for(int i=0; i<N; i++)
        {
            organisms[i] = Organism();
        }

        bestOrgIdx = -1;
        findBestOrganism();
    }

    void performMutualism(int x_i)
    {
		int x_j = x_i;
        while(x_j == x_i) x_j = rand()%N;

        pair<float,float> mutualVector = {(organisms[x_i].alpha+organisms[x_j].alpha)/2, (organisms[x_i].beta+organisms[x_j].beta)/2};
        int BF1 = rand()%2+1;
        int BF2 = rand()%2+1;
        float rand1 = rand()/(float)RAND_MAX;
        float rand2 = rand()/(float)RAND_MAX;
		float org1Alpha = organisms[x_i].alpha;
		float org1Beta = organisms[x_i].beta;
		float org2Alpha = organisms[x_j].alpha;
		float org2Beta = organisms[x_j].beta;
		
		organisms[x_i].alpha = organisms[x_i].alpha + rand1*(organisms[bestOrgIdx].alpha - mutualVector.first*BF1);
		organisms[x_i].beta = organisms[x_i].beta + rand1*(organisms[bestOrgIdx].beta - mutualVector.second*BF1);
		float newFitnessValue = 1/organisms[x_i].startACO(false);
		if(newFitnessValue<organisms[x_i].fitnessValue){
			organisms[x_i].alpha = org1Alpha;
			organisms[x_i].beta = org1Beta;
		}

		organisms[x_j].alpha = organisms[x_j].alpha + rand2*(organisms[bestOrgIdx].alpha - mutualVector.first*BF2);
		organisms[x_j].beta = organisms[x_j].beta + rand2*(organisms[bestOrgIdx].beta - mutualVector.second*BF2);
		newFitnessValue = 1/organisms[x_j].startACO(false);
		if(newFitnessValue<organisms[x_j].fitnessValue){
			organisms[x_j].alpha = org2Alpha;
			organisms[x_j].beta = org2Beta;
		}
    }	

	void performCommensalism(int x_i)
	{
		int x_j = x_i;
        while(x_j == x_i) x_j = rand()%N;

        float org1Alpha = organisms[x_i].alpha, org1Beta = organisms[x_i].beta;
        float randNum = rand()/(float) RAND_MAX + rand()%2 - 1; // (-1, 1)
        
        organisms[x_i].alpha = organisms[x_i].alpha + randNum * (organisms[bestOrgIdx].alpha - organisms[x_j].alpha);
        organisms[x_i].beta = organisms[x_i].beta + randNum * (organisms[bestOrgIdx].beta - organisms[x_j].beta);

        float newFitnessValue = 1/organisms[x_i].startACO(false);
		if(newFitnessValue<organisms[x_i].fitnessValue){
			organisms[x_i].alpha = org1Alpha;
			organisms[x_i].beta = org1Beta;
		}
	}

	void performParasitism(int x_i)
	{
		int x_j = x_i;
        while(x_j == x_i) x_j = rand()%N;

        float org2Alpha = organisms[x_j].alpha, org2Beta = organisms[x_j].beta;
        float randNum = rand()/(float)RAND_MAX;
        
        if(randNum < 0.5)
        {
            // Update alpha
            organisms[x_j].alpha = organisms[x_i].alpha + rand()/(float)RAND_MAX+rand()%2-1;
        }
        else 
        {
            // Update beta
            organisms[x_j].beta = organisms[x_i].beta + rand()/(float)RAND_MAX+rand()%2-1;
        }

        float newFitnessValue = 1/organisms[x_j].startACO(false);
		if(newFitnessValue<organisms[x_j].fitnessValue){
			organisms[x_j].alpha = org2Alpha;
			organisms[x_j].beta = org2Beta;
		}
	}

    void findBestOrganism() { 
        float maxFitness = 0;
		for(int i=0; i<N; i++)
        {
            float fitnessVal = organisms[i].fitnessValue;
            if(fitnessVal > maxFitness)
            {
                maxFitness = fitnessVal;
                bestOrgIdx = i;
            }
		}  
	}

    Organism performSOSACO() 
    {
        for(int j=0; j<ITER_MAX; j++)
        {
            // Perform SOS
            for(int i=0; i<N; i++)
            {
                performMutualism(i);
                performCommensalism(i);
                performParasitism(i);
				// findBestOrganism();
            }

            for(int i=0; i<N; i++)
            {
                float distance = organisms[i].startACO(true);
				organisms[i].fitnessValue = 1/distance;
            }
            
			findBestOrganism();
			// cout<<"iteration: "<<j<<"\n";
			// for(int i=0;i<N;i++)  cout<<organisms[i].fitnessValue<<" ";
			// cout<<"\n";
        }
		return organisms[bestOrgIdx];
    }
};

int main(int argc, char *argv[]) {
    srand(time(0));

    if(argc==0) return 1;
    string filename = argv[1];
    ITER_MAX = atoi(argv[2]);

    // cout<<"Dataset: "<<filename<<"\n";
    // cout<<"Number of Iterations: "<<ITER_MAX<<"\n";

    parseData(filename);
    HybridSOSACO SOS = HybridSOSACO();
    Organism bestOrg = SOS.performSOSACO();

    cout<< bestOrg.bestDistance;
	// for(int i=0;i<SOS.organisms.size();i++) cout<<SOS.organisms[i].alpha<<":"<<SOS.organisms[i].beta<<"\n";
	
}