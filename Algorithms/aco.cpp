#include <bits/stdc++.h>
#include <cstdio>
#include <iostream>
using namespace std;

const float RHO = 0.1;
const float QNOT = 0.99;
const float BETA = 5.00;
const float ALPHA = 1.00;
const float GAMMA = 0.90;
const float PHI = 0.1;
const int ANT_COUNT = 100;
int MAX_ITERATIONS = 300;
const float QVAL = 1000.00;
const float MAX_PHEROMONE = 300.0;
const float TAU_NOT = 1.00;
const float QMIN = 0.001;

struct City
{
    float x;
    float y;
};

struct Edge
{
    float distance;
    float pheromone;
};

class AntColonyOptimization
{
public:
    int numCities;
    float optimalDistance;
    vector<City> cities;
    vector<vector<Edge>> edges;
    vector<int> optimalPath;

    AntColonyOptimization(string filename)
    {
        optimalDistance = INT_MAX;
        parseData(filename);
        init();
        start();
    }

    float euclideanDistance(City city1, City city2)
    {
        return pow(pow(city1.x - city2.x, 2) + pow(city1.y - city2.y, 2), 0.5);
    }

    void parseData(string filename)
    {
        freopen(filename.c_str(), "r", stdin);
        int id;

        while (cin >> id)
        {
            float x, y;
            City city;
            cin >> city.x >> city.y;
            cities.push_back(city);
        }
    }

    void init()
    {
        numCities = cities.size();
        edges = vector<vector<Edge>>(numCities, vector<Edge>(numCities));
        for (int i = 0; i < numCities; i++)
        {
            for (int j = i + 1; j < numCities; j++)
            {
                edges[i][j].distance = euclideanDistance(cities[i], cities[j]);
                edges[i][j].pheromone = TAU_NOT; // edges[i][j].distance;
                edges[j][i] = edges[i][j];
            }
        }
    }

    void start()
    {
        for (int i = 0; i < MAX_ITERATIONS; i++)
        {
            float bestIterDistance = INT_MAX;
            vector<int> bestIterPath;
            for (int ant = 0; ant < ANT_COUNT; ant++)
            {
                pair<float, vector<int>> path = tour();
                float antTourDistance = path.first;

                vector<int> antTourPath = path.second;
                if (antTourDistance < optimalDistance)
                {
                    optimalDistance = antTourDistance;
                    // cout<<"dist: "<<optimalDistance<<"\n";
                    optimalPath = antTourPath;
                }
                if (antTourDistance < bestIterDistance)
                {
                    bestIterDistance = antTourDistance;
                    bestIterPath = antTourPath;
                }

                normalize();
            }
            globalUpdate(optimalPath, optimalDistance);
            normalize();
        }
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
            localUpdate(currentCity, nextCity);
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

        if (q < QNOT)
        {
            // pheromone dependent
            float maxProbability = INT_MIN;
            for (int allowedCity = 0; allowedCity < numCities; allowedCity++)
            {
                if (!visited[allowedCity])
                {
                    float pheromone = edges[currentCity][allowedCity].pheromone;
                    float distance = edges[currentCity][allowedCity].distance;
                    float probability = pow(pheromone, ALPHA) * pow(1 / distance, BETA);
                    if (probability > maxProbability)
                    {
                        maxProbability = probability;
                        nextCity = allowedCity;
                    }
                }
            }
        }
        else
        {
            // random select of next not visited node
            vector<int> allowedCities;
            for (int allowedCity = 0; allowedCity < numCities; allowedCity++)
            {
                if (!visited[allowedCity])
                    allowedCities.push_back(allowedCity);
            }
            int idx = rand() % allowedCities.size();
            nextCity = allowedCities[idx];
        }

        return nextCity;
    }

    void localUpdate(int city1, int city2)
    {
        float newPheromone = (1 - PHI) * edges[city1][city2].pheromone + (PHI * TAU_NOT);
        if (newPheromone < QMIN)
            newPheromone = QMIN;
        edges[city1][city2].pheromone = edges[city2][city1].pheromone = newPheromone;
    }

    void globalUpdate(vector<int> path, float totalDistance)
    {
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

        for (int i = 1; i < path.size(); i++)
        {
            int currentCity = path[i - 1];
            int nextCity = path[i];
            int distance = edges[currentCity][nextCity].distance;
            edges[currentCity][nextCity].pheromone += (QVAL / totalDistance); // * distance;
            edges[nextCity][currentCity].pheromone += (QVAL / totalDistance); // * distance;
        }
    }

    void normalize()
    {
        float totalPheromone = 0.0;
        for (int i = 0; i < numCities; i++)
        {
            for (int j = i + 1; j < numCities; j++)
            {
                totalPheromone += edges[i][j].pheromone;
            }
        }

        if (totalPheromone > MAX_PHEROMONE)
        {
            float normalizedFactor = MAX_PHEROMONE / totalPheromone;
            for (int i = 0; i < numCities; i++)
            {
                for (int j = i + 1; j < numCities; j++)
                {
                    float normalizedPheromone = normalizedFactor * edges[i][j].pheromone;
                    edges[i][j].pheromone = edges[j][i].pheromone = normalizedPheromone;
                }
            }
        }
    }

    float getOptimalDistance()
    {
        return optimalDistance;
    }

    vector<int> getOptimalPath()
    {
        return optimalPath;
    }
};

int main(int argc, char *argv[])
{

    srand(time(0));

    if(argc==0) return 1;
    string filename = argv[1];
    MAX_ITERATIONS = atoi(argv[2]);

    // cout<<"Dataset: "<<filename<<"\n";
    // cout<<"Number of Iterations: "<<ITER_MAX<<"\n";

    AntColonyOptimization ACO = AntColonyOptimization(filename);
    float distance = ACO.getOptimalDistance();

    cout<<distance;
	// for(int i=0;i<SOS.organisms.size();i++) cout<<SOS.organisms[i].alpha<<":"<<SOS.organisms[i].beta<<"\n";
	
}