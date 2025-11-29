#ifndef _SAINT_VENANT_H
#define _SAINT_VENANT_H

#include <Eigen/Dense>
#include <vector>
#include <string>
#include <fstream>

// Structure pour l'état W = (h, hu)
struct State {
    double h;   // hauteur d'eau
    double hu;  // débit
    
    State(double h_ = 0.0, double hu_ = 0.0) : h(h_), hu(hu_) {}
    
    double u() const { 
        return (h > 1e-10) ? hu/h : 0.0; 
    }
    
    // Opérateurs pour faciliter les calculs
    State operator+(const State& other) const {
        return State(h + other.h, hu + other.hu);
    }
    
    State operator-(const State& other) const {
        return State(h - other.h, hu - other.hu);
    }
    
    State operator*(double scalar) const {
        return State(h * scalar, hu * scalar);
    }
};

inline State operator*(double scalar, const State& s) {
    return s * scalar;
}

// ===============================================
// Classe pour les flux numériques
// ===============================================
class NumericalFlux
{
protected:
    static constexpr double _g = 9.81; // gravité
    
public:
    NumericalFlux();
    virtual ~NumericalFlux();
    
    // Flux physique F(W)
    State PhysicalFlux(const State& W) const;
    
    // Vitesse maximale de propagation
    double MaxWaveSpeed(const State& W) const;
    
    // Flux numérique (à implémenter dans les classes filles)
    virtual State ComputeFlux(const State& WL, const State& WR) const = 0;
};

// Flux de Lax-Friedrichs
class LaxFriedrichsFlux : public NumericalFlux
{
public:
    State ComputeFlux(const State& WL, const State& WR) const override;
};

// Flux de Rusanov
class RusanovFlux : public NumericalFlux
{
public:
    State ComputeFlux(const State& WL, const State& WR) const override;
};

// ===============================================
// Classe pour les schémas en temps
// ===============================================
class SaintVenantSolver
{
protected:
    int _N;                          // Nombre de cellules
    double _L;                       // Longueur du domaine
    double _dx;                      // Pas d'espace
    double _dt;                      // Pas de temps
    double _t;                       // Temps actuel
    double _CFL;                     // Nombre CFL
    std::vector<State> _W;           // Solution
    NumericalFlux* _flux;            // Pointeur vers le flux numérique
    std::ofstream _file_out;         // Fichier de sortie
    
public:
    SaintVenantSolver();
    virtual ~SaintVenantSolver();
    
    // Initialisation
    void Initialize(int N, double L, double CFL, const std::string& results, 
                   NumericalFlux* flux);
    
    // Condition initiale
    void SetInitialCondition(const std::vector<State>& W0);
    
    // Calcul du pas de temps CFL
    double ComputeTimeStep();
    
    // Avancer d'un pas de temps (à implémenter)
    virtual void Advance() = 0;
    
    // Sauvegarde de la solution
    void SaveSolution();
    
    // Accesseurs
    const std::vector<State>& GetSolution() const { return _W; }
    double GetTime() const { return _t; }
    double GetDx() const { return _dx; }
    double GetDt() const { return _dt; }
};

// Schéma de Godunov
class GodunovScheme : public SaintVenantSolver
{
public:
    void Advance() override;
};

// ===============================================
// Classe pour les conditions initiales
// ===============================================
class InitialCondition
{
public:
    static std::vector<State> DamBreak(int N, double x_dam);
    static std::vector<State> SineWave(int N, double amplitude);
    static std::vector<State> Uniform(int N, double h0, double u0);
};

#endif // _SAINT_VENANT_H