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

void ejecutarRR(vector<Proceso>& procesos, double Q) {
    sort(procesos.begin(), procesos.end(), [](const Proceso& a, const Proceso& b) {
        return a.ti < b.ti;
    });

    int n = procesos.size();
    queue<int> cola;
    double tiempo_actual = 0;
    int idx_ingreso = 0;
    int completados = 0;

    if (n > 0) {
        tiempo_actual = procesos[0].ti;
        cola.push(0);
        idx_ingreso = 1;
    }

    while (completados < n) {
        if (cola.empty()) {
            if (idx_ingreso < n) {
                tiempo_actual = procesos[idx_ingreso].ti;
                cola.push(idx_ingreso);
                idx_ingreso++;
            }
            continue;
        }

        int i = cola.front();
        cola.pop();

        double tiempo_ejecucion = min(Q, procesos[i].t_restante);
        tiempo_actual += tiempo_ejecucion;
        procesos[i].t_restante -= tiempo_ejecucion;

        while (idx_ingreso < n && procesos[idx_ingreso].ti <= tiempo_actual) {
            cola.push(idx_ingreso);
            idx_ingreso++;
        }

        if (procesos[i].t_restante > 0) {
            cola.push(i);
        } else {
            procesos[i].tf = tiempo_actual;
            completados++;
        }
    }
}


void escribirResultados(ofstream& out, const string& nombre_metodo, vector<Proceso>& procesos, 
                        double prom_T, double prom_E, double prom_I, long long microsegundos) {
    out << "========================================================================\n";
    out << "METODO: " << nombre_metodo << "\n";
    out << "Tiempo de calculo del algoritmo: " << microsegundos << " microsegundos\n";
    out << "------------------------------------------------------------------------\n";
    out << left << setw(5) << "ID" << setw(10) << "ti" << setw(10) << "t" 
        << setw(10) << "tf" << setw(10) << "T" << setw(10) << "E" << setw(10) << "I" << "\n";
    

    sort(procesos.begin(), procesos.end(), [](const Proceso& a, const Proceso& b) {
        return a.id < b.id;
    });

    for (const auto& p : procesos) {
        out << left << setw(5) << p.id 
            << setw(10) << p.ti << setw(10) << p.t 
            << setw(10) << p.tf << setw(10) << p.T 
            << setw(10) << p.E << setw(10) << fixed << setprecision(4) << p.I << "\n";
    }
    out << "------------------------------------------------------------------------\n";
    out << "Promedios -> T: " << prom_T << " | E: " << prom_E << " | I: " << prom_I << "\n\n";
}


int main() {
    vector<Proceso> procesos;
    ifstream archivo("datos.txt");
    
    if (!archivo.is_open()) {
        cout << "Error: No se encontro 'datos.txt'." << endl;
        return 1;
    }

    string linea;
    double quantum;

    cout << "Ingrese el valor del Quantum (Q) para Round Robin: ";
    cin >> quantum;


    while (getline(archivo, linea)) {
  
        for (char& c : linea) {
            if (c == '(' || c == ')' || c == '\t' || c == ' ') {
                c = ',';
            }
        }
        
        stringstream ss(linea);
        string item;
        vector<string> tokens;
        
        while (getline(ss, item, ',')) {
            if (!item.empty()) { 
                tokens.push_back(item);
            }
        }
        
       
        if (tokens.size() >= 3) {
            Proceso p;
            p.id = tokens[0];
            p.ti = stod(tokens[1]);
            p.t = stod(tokens[2]);
            procesos.push_back(p);
        }
    }
    archivo.close();

    if(procesos.empty()) {
        cout << "Error: No se leyeron procesos. Revisa el formato de datos.txt" << endl;
        return 1;
    }