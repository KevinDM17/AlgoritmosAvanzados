/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: Antonio
 *
 * Created on June 24, 2024, 4:09 PM
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <cstring>
#include <fstream>
#include <map>
#include <iterator>

#define CORTE 6
#define MAX_PLAYERS 428
#define MAX_POBLACION 1000

using namespace std;

struct Player {
    string full_name;
    int rating;
    int jersey;
    string team;
    int position; // in numbers
    int age; // set on a specific date
    double height; // in meters
    double weight; // in kilograms
    double salary; // in dollars
    string country;
    int draft_year;
    int draft_round; // 0 means undrafted
    int draft_peak; // 0 means undrafted
    string college;
};

struct Individual {
    vector<int> chromosome;
    int fitness;
    double investment;

    // constructor
    Individual(vector<int> chromo) : chromosome(chromo), fitness(0) {}
};

int getNumericPosition(string position_character, map<string, int>& posiciones, int& totalPosiciones) {
    for(auto recorrer : posiciones){
        if(position_character=="G-F")
            position_character="F-G";
        if(position_character=="C-F")
            position_character="F-C";
        if(position_character==recorrer.first){
            return recorrer.second;
        }
    }
    posiciones[position_character]=totalPosiciones;
    totalPosiciones++;
    return totalPosiciones-1;
}

int calculateAge(int birth_day, int birth_month, int birth_year) {
    birth_year = (birth_year > 50) ? birth_year + 1900 : birth_year + 2000;
    int today_day = 24;
    int today_month = 6;
    int today_year = 2024;

    int age = today_year - birth_year;

    if (today_month < birth_month || (today_month == birth_month && today_day < birth_day)) {
        age--;
    }
    return age;
}

void loadPlayers(vector<Player>& listOfPlayers, map<string, int>& posiciones, int& totalPosiciones) {
    ifstream arch("nba2k20-full.csv");
    if (!arch.is_open()) {
        cout << "ERROR: No se pudo abrir el archivo" << endl;
        exit(1);
    }

    Player player_readed;
    char string_aux[100], car;
    int dd, mm, aa;
    double doble_aux;

    // Skip header
    while (arch.get() != '\n');

    while (true) {
        arch.getline(string_aux, 100, ','); // full_name
        if (arch.eof()) break;
        player_readed.full_name = string_aux;
        arch >> player_readed.rating >> car >> car >> player_readed.jersey >> car;
        arch.getline(string_aux, 100, ','); // team
        player_readed.team = string_aux;
        arch.getline(string_aux, 100, ','); // position
        player_readed.position = getNumericPosition(string_aux, posiciones, totalPosiciones);
        arch >> dd >> car >> mm >> car >> aa >> car;
        player_readed.age = calculateAge(dd, mm, aa);
        arch >> doble_aux >> car >> doble_aux >> car >> car >> car;
        arch >> player_readed.height >> car >> player_readed.weight;
        arch.getline(string_aux, 100, '$');
        arch >> player_readed.salary >> car;
        arch.getline(string_aux, 100, ',');
        player_readed.country = string_aux;
        arch >> player_readed.draft_year >> car >> player_readed.draft_round >> car >> player_readed.draft_peak >> car;
        arch.getline(string_aux, 100, '\n');
        player_readed.college = string_aux;

        listOfPlayers.push_back(player_readed);
    }
}

void printPlayers(const vector<Player>& listOfPlayers) {
    for (const auto& player : listOfPlayers) {
        cout << left << setw(50) << player.full_name << player.age << " " << player.position << endl;
    }
}

int calculate_fitness(const vector<int>& chromosome, const vector<Player>& playerPool, 
        double max_budget, int totalPosiciones) {
    int total_overall = 0;
    int total_investment = 0;
    vector<int> position_count(totalPosiciones, 0);
    int total_selected = 0;

    for (int i = 0; i < chromosome.size(); ++i) {
        if (chromosome[i] == 1) {
            total_overall += playerPool[i].rating;
            total_investment+=playerPool[i].salary;
            position_count[playerPool[i].position]++;
            total_selected++;
        }
    }

    bool valid_team = total_selected == 10;
    for (int count : position_count) {
        if (count > 2) {
            valid_team = false;
            break;
        }
    }
    return (valid_team and total_investment<=max_budget) ? static_cast<double>(total_overall) : 0;
}

vector<Individual> init_population(int population_size, int chromosome_length, 
        const vector<Player>& playerPool, int totalPosiciones, double max_investment) {
    vector<Individual> population;
    srand(time(0)); // Initialize random seed
    for (int i = 0; i < population_size; ++i) {
        double inversionEquipo=0;
        vector<int> chromosome(chromosome_length, 0);
        int num_selected = 0;
        while (num_selected < 10) {
            int idx = rand() % chromosome_length;
            if (chromosome[idx] == 0) {
                chromosome[idx] = 1;
                inversionEquipo+=playerPool[idx].salary;
                num_selected++;
            }
        }
        double fitness = calculate_fitness(chromosome, playerPool, 
                max_investment, totalPosiciones);
        if (fitness > 0 and inversionEquipo<=max_investment) {
            population.push_back(Individual(chromosome));
            population.back().fitness = fitness;
        } else {
            i--; // Retry if invalid chromosome
        }
    }
    return population;
}

double calculate_investment(vector <int>chromosome,vector<Player>listOfPlayers){
    double total=0;
    for(int i=0; i<chromosome.size(); i++){
        if(chromosome[i]==1){
            total+=listOfPlayers[i].salary;
        }
    }
    return total;
}

void evaluarPoblacion(vector<Individual>& population, 
        const vector<Player>& listOfPlayers, int totalPosiciones, double max_investment) {
    for (auto& individuo : population) {
        individuo.fitness = calculate_fitness(individuo.chromosome, 
                listOfPlayers, max_investment, totalPosiciones);
        individuo.investment=calculate_investment(individuo.chromosome,listOfPlayers);
    }
}

Individual tournament_selection(vector<Individual>& population, int tournament_size) {
    vector<Individual> tournament;
    for (int i = 0; i < tournament_size; ++i) {
        int random_index = rand() % population.size();
        tournament.push_back(population[random_index]);
    }

    return *max_element(tournament.begin(), tournament.end(), [](Individual& a, Individual& b) {
        return a.fitness < b.fitness;
    });
}
pair<Individual, Individual> crossover_uniform(Individual& parent1, Individual& parent2) {
    int size = parent1.chromosome.size();
    vector<int> child1_chromo(size);
    vector<int> child2_chromo(size);

    for (int i = 0; i < size; ++i) {
        if (rand() % 2) {
            child1_chromo[i] = parent1.chromosome[i];
            child2_chromo[i] = parent2.chromosome[i];
        } else {
            child1_chromo[i] = parent2.chromosome[i];
            child2_chromo[i] = parent1.chromosome[i];
        }
    }

    return make_pair(Individual(child1_chromo), Individual(child2_chromo));
}

pair<Individual, Individual> crossover_custom(Individual& parent1, Individual& parent2) {
    int size = parent1.chromosome.size();
    vector<int> child1_chromo(size);
    vector<int> child2_chromo(size);

    for (int i = 0; i < CORTE; ++i) {
        child1_chromo[i] = parent1.chromosome[i];
    }
    for (int i = CORTE; i < size; ++i) {
        child1_chromo[i] = parent2.chromosome[i];
    }

    for (int i = 0; i < CORTE; ++i) {
        child2_chromo[i] = parent2.chromosome[i];
    }
    for (int i = CORTE; i < size; ++i) {
        child2_chromo[i] = parent1.chromosome[i];
    }

    return make_pair(Individual(child1_chromo), Individual(child2_chromo));
}

void mutation_flip(Individual& ind) {
    int position = rand() % ind.chromosome.size();
    ind.chromosome[position] = 1 - ind.chromosome[position];
}

vector<Individual> select_survivors_ranking(vector<Individual>& population, 
        vector<Individual>& offspring_population, int numsurvivors) {
    vector<Individual> next_population;
    population.insert(population.end(), offspring_population.begin(), offspring_population.end());
    sort(population.begin(), population.end(), [](Individual& a, Individual& b) {
        return a.fitness > b.fitness;
    });
    for (int i = 0; i < numsurvivors; ++i) {
        next_population.push_back(population[i]);
    }
    return next_population;
}

void algoritmoGenetico(const vector<Player>& listOfPlayers, vector<Individual>& population, 
        int generaciones, int totalPosiciones, int tournament_size, double mutation_rate,
        map <string, int>posiciones, double max_investment) {
    vector<int> bestfitness;
    evaluarPoblacion(population, listOfPlayers, totalPosiciones, max_investment);
    auto best_individual = max_element(population.begin(), population.end(), 
                [](const Individual& a, const Individual& b) {return a.fitness < b.fitness;
    });
    bestfitness.push_back(best_individual->fitness);
    cout << "Poblacion inicial, best_fitness = " << best_individual->fitness << endl;
    for (int g = 0; g < generaciones; ++g) {
        
        // crea las parejas a cruzarse (mating pool)
        vector<pair<Individual, Individual>> mating_pool;
        for (int i = 0; i < MAX_POBLACION / 2; ++i) {
            mating_pool.push_back(make_pair(tournament_selection(population, tournament_size), 
                    tournament_selection(population, tournament_size)));
        }

        // cruza las parejas del mating pool. Cada cruzamiento genera 2 hijos
        vector<Individual> offspring_population;
        for (auto& parents : mating_pool) {  // por cada pareja del mating pool
             pair<Individual, Individual> children = crossover_uniform(parents.first, parents.second); // cruzamiento one point
            //pair<Individual, Individual> children = crossover_custom(parents.first, parents.second);  // cruzamiento uniforme

            if ((double)rand() / RAND_MAX < mutation_rate) { // intenta mutar el hijo 1 de acuerdo a la tasa de mutacion
                mutation_flip(children.first);
            }
            if ((double)rand() / RAND_MAX < mutation_rate) { // intenta mutar el hijo 2 de acuerdo a la tasa de mutacion
                mutation_flip(children.second);
            }
            offspring_population.push_back(children.first);  // agrega el hijo 1 a la poblacion descendencia
            offspring_population.push_back(children.second); // agrega el hijo 2 a la poblacion descendencia
        }

        evaluarPoblacion(offspring_population, listOfPlayers, totalPosiciones,max_investment); // evalua poblacion descendencia
        population = select_survivors_ranking(population, offspring_population, MAX_POBLACION); // selecciona sobrevivientes por ranking

        // obtiene el mejor individuo de la poblacion sobreviviente
        best_individual = max_element(population.begin(), population.end(), [](Individual& a, Individual& b) {
            return a.fitness < b.fitness;
        });
        bestfitness.push_back(best_individual->fitness);

        if (g % 2 == 0) { // reporta cada 2 generaciones
            cout << "Generation " << g << ", Best fitness: " << best_individual->fitness << endl;
        }
    }
    cout << "Mejor individuo en la ultima generacion: [";
    for (int gene : best_individual->chromosome) {
        cout << gene << " ";
    }
    cout << "] con fitness: " << best_individual->fitness << endl;
    int i=0;
    cout.precision(2);
    cout<<fixed;
    cout<<"==========================================DREAM TEAM====================="
            "====================="<<endl;
    for (int gene : best_individual->chromosome) {
        if(gene==1){
            cout<<left<<setw(40)<<listOfPlayers[i].full_name<<setw(10)<<listOfPlayers[i].rating;
            for(auto it : posiciones)
                if(it.second==listOfPlayers[i].position)
                    cout<<left<<setw(20)<<it.first;
            cout<<right<<setw(12)<<listOfPlayers[i].salary<<endl;
        }
        i++;
    }
    cout << "Inversion total: "<<calculate_investment(best_individual->chromosome,listOfPlayers);
}

int main(int argc, char** argv) {
    vector<Player> listOfPlayers;
    map<string, int> posiciones;
    int totalPosiciones = 0;
    const int TOURNAMENT_SIZE = 3;
    const double MUTATION_RATE = 0.8;
    const double max_investment=200000000;
    loadPlayers(listOfPlayers, posiciones, totalPosiciones);

    if (listOfPlayers.empty()) {
        cout << "No players loaded. Exiting." << endl;
        return 0;
    }
    
    int population_size = MAX_POBLACION;
    vector<Individual> initial_population = init_population(population_size, 
            listOfPlayers.size(), listOfPlayers, totalPosiciones, max_investment);

    if (initial_population.empty()) {
        cout << "Failed to initialize population. Exiting." << endl;
        return 0;
    }

    int generaciones = 100;
    algoritmoGenetico(listOfPlayers, initial_population, generaciones, 
                    totalPosiciones,TOURNAMENT_SIZE,MUTATION_RATE,
                    posiciones, max_investment);
    return 0;
}

