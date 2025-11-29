#ifndef _SAINT_VENANT_CPP
#define _SAINT_VENANT_CPP

#include "SaintVenant.h"
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

// ===============================================
// Implémentation NumericalFlux
// ===============================================
NumericalFlux::NumericalFlux() {}

NumericalFlux::~NumericalFlux() {}

State NumericalFlux::PhysicalFlux(const State& W) const {
    double h = W.h;
    double hu = W.hu;
    double u = W.u();
    
    State F;
    F.h = hu;
    F.hu = hu*u + 0.5*_g*h*h;
    return F;
}

double NumericalFlux::MaxWaveSpeed(const State& W) const {
    if (W.h < 1e-10) return 0.0;
    double u = W.u();
    double c = sqrt(_g * W.h); // célérité
    return fabs(u) + c;
}

// ===============================================
// Flux de Lax-Friedrichs
// ===============================================
State LaxFriedrichsFlux::ComputeFlux(const State& WL, const State& WR) const {
    State FL = PhysicalFlux(WL);
    State FR = PhysicalFlux(WR);
    
    double alphaL = MaxWaveSpeed(WL);
    double alphaR = MaxWaveSpeed(WR);
    double alpha = max(alphaL, alphaR);
    
    State F_LF;
    F_LF.h = 0.5 * (FL.h + FR.h - alpha * (WR.h - WL.h));
    F_LF.hu = 0.5 * (FL.hu + FR.hu - alpha * (WR.hu - WL.hu));
    
    return F_LF;
}

// ===============================================
// Flux de Rusanov
// ===============================================
State RusanovFlux::ComputeFlux(const State& WL, const State& WR) const {
    State FL = PhysicalFlux(WL);
    State FR = PhysicalFlux(WR);
    
    double alphaL = MaxWaveSpeed(WL);
    double alphaR = MaxWaveSpeed(WR);
    double alpha = max(alphaL, alphaR);
    
    State F_Rus;
    F_Rus.h = 0.5 * (FL.h + FR.h - alpha * (WR.h - WL.h));
    F_Rus.hu = 0.5 * (FL.hu + FR.hu - alpha * (WR.hu - WL.hu));
    
    return F_Rus;
}

// ===============================================
// Implémentation SaintVenantSolver
// ===============================================
SaintVenantSolver::SaintVenantSolver() : _flux(nullptr), _t(0.0) {}

SaintVenantSolver::~SaintVenantSolver() {
    if (_file_out.is_open()) {
        _file_out.close();
    }
}

void SaintVenantSolver::Initialize(int N, double L, double CFL, 
                                   const string& results, NumericalFlux* flux) {
    _N = N;
    _L = L;
    _dx = L / N;
    _CFL = CFL;
    _flux = flux;
    _t = 0.0;
    
    _W.resize(N);
    
    if (!results.empty()) {
        _file_out.open(results);
    }
}

void SaintVenantSolver::SetInitialCondition(const vector<State>& W0) {
    _W = W0;
}

double SaintVenantSolver::ComputeTimeStep() {
    double maxSpeed = 0.0;
    for (const auto& w : _W) {
        maxSpeed = max(maxSpeed, _flux->MaxWaveSpeed(w));
    }
    return (maxSpeed > 0) ? _CFL * _dx / maxSpeed : 0.01;
}

void SaintVenantSolver::SaveSolution() {
    for (int i = 0; i < _N; i++) {
        double x = (i + 0.5) * _dx;
        _file_out << _t << " " << x << " " 
                  << _W[i].h << " " << _W[i].u() << endl;
    }
    _file_out << endl; // Ligne vide pour gnuplot
}

// ===============================================
// Schéma de Godunov
// ===============================================
void GodunovScheme::Advance() {
    vector<State> W_new(_N);
    
    // Calcul du pas de temps
    _dt = ComputeTimeStep();
    
    // Calcul des flux aux interfaces
    for (int i = 1; i < _N-1; i++) {
        // Flux à droite F_{i+1/2}
        State F_right = _flux->ComputeFlux(_W[i], _W[i+1]);
        
        // Flux à gauche F_{i-1/2}
        State F_left = _flux->ComputeFlux(_W[i-1], _W[i]);
        
        // Mise à jour conservative
        double coeff = _dt / _dx;
        W_new[i].h = _W[i].h - coeff * (F_right.h - F_left.h);
        W_new[i].hu = _W[i].hu - coeff * (F_right.hu - F_left.hu);
    }
    
    // Conditions aux limites (réflexion)
    W_new[0] = W_new[1];
    W_new[_N-1] = W_new[_N-2];
    
    _W = W_new;
    _t += _dt;
}

// ===============================================
// Conditions initiales
// ===============================================
vector<State> InitialCondition::DamBreak(int N, double x_dam) {
    vector<State> W(N);
    double dx = 1.0 / N;
    
    for (int i = 0; i < N; i++) {
        double x = (i + 0.5) * dx;
        if (x < x_dam) {
            W[i] = State(2.0, 0.0); // Eau haute à gauche
        } else {
            W[i] = State(0.5, 0.0); // Eau basse à droite
        }
    }
    return W;
}

vector<State> InitialCondition::SineWave(int N, double amplitude) {
    vector<State> W(N);
    double dx = 1.0 / N;
    
    for (int i = 0; i < N; i++) {
        double x = (i + 0.5) * dx;
        double h = 1.0 + amplitude * sin(2.0 * M_PI * x);
        W[i] = State(h, 0.0);
    }
    return W;
}

vector<State> InitialCondition::Uniform(int N, double h0, double u0) {
    vector<State> W(N);
    for (int i = 0; i < N; i++) {
        W[i] = State(h0, h0 * u0);
    }
    return W;
}

#endif // _SAINT_VENANT_CPP