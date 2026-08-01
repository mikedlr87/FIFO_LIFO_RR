#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;

struct Proceso {
    string id;
    double ti; 
    double t;  
    
    double tf; 
    double T;  
    double E;  
    double I;  
    
    double t_restante; 
    bool completado;
};

void reiniciarProcesos(vector<Proceso>& procesos) {
    for (auto& p : procesos) {
        p.t_restante = p.t;
        p.completado = false;
        p.tf = 0; p.T = 0; p.E = 0; p.I = 0;
    }
}

void calcularMetricas(vector<Proceso>& procesos, double& prom_T, double& prom_E, double& prom_I) {
    prom_T = 0; prom_E = 0; prom_I = 0;
    if (procesos.empty()) return; // Prevenir división por cero
    
    for (auto& p : procesos) {
        p.T = p.tf - p.ti;
        p.E = p.T - p.t;
        p.I = p.t / p.T;
        
        prom_T += p.T;
        prom_E += p.E;
        prom_I += p.I;
    }
    int n = procesos.size();
    prom_T /= n; prom_E /= n; prom_I /= n;
}

void ejecutarFIFO(vector<Proceso>& procesos) {
    sort(procesos.begin(), procesos.end(), [](const Proceso& a, const Proceso& b) {
        return a.ti < b.ti;
    });

    double tiempo_actual = 0;
    for (auto& p : procesos) {
        if (tiempo_actual < p.ti) tiempo_actual = p.ti;
        tiempo_actual += p.t;
        p.tf = tiempo_actual;
    }
}

void ejecutarLIFO(vector<Proceso>& procesos) {
    int n = procesos.size();
    int completados = 0;
    double tiempo_actual = 0;

    while (completados < n) {
        int idx_seleccionado = -1;
        double max_ti_disponible = -1;
        double min_ti_futuro = 1e9;

        for (int i = 0; i < n; i++) {
            if (!procesos[i].completado) {
                if (procesos[i].ti <= tiempo_actual) {
                    if (procesos[i].ti > max_ti_disponible) {
                        max_ti_disponible = procesos[i].ti;
                        idx_seleccionado = i;
                    }
                } else {
                    if (procesos[i].ti < min_ti_futuro) {
                        min_ti_futuro = procesos[i].ti;
                    }
                }
            }
        }

        if (idx_seleccionado != -1) {
            tiempo_actual += procesos[idx_seleccionado].t;
            procesos[idx_seleccionado].tf = tiempo_actual;
            procesos[idx_seleccionado].completado = true;
            completados++;
        } else {
            tiempo_actual = min_ti_futuro;
        }
    }
}

