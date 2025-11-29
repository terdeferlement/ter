// godunov_hll_gnuplot.cpp
#include <bits/stdc++.h>
using namespace std;

// Constante physique
const double g = 9.81;

// Paramètres numériques
struct Params {
    int nx = 400;        // nombre de mailles
    double xL = 0.0;
    double xR = 10.0;
    double CFL = 0.9;
    double tFinal = 1.0;
    double hmin = 1e-8;  // plancher pour éviter h<0
};

// État conservatif W = [h, hu]
struct State {
    double h, hu;
    State() : h(0), hu(0) {}
    State(double _h, double _hu) : h(_h), hu(_hu) {}
};

// Flux physique F(W)
State flux(const State &W) {
    State F;
    double h = W.h;
    double hu = W.hu;
    double u = (h > 0.0) ? hu / h : 0.0;
    F.h = hu;
    F.hu = (h > 0.0) ? hu * u + 0.5 * g * h * h : 0.0;
    return F;
}

// Vitesse d’onde max pour CFL
double max_wave_speed(const State &W) {
    if (W.h <= 0.0) return 0.0;
    double u = W.hu / W.h;
    return fabs(u) + sqrt(g * W.h);
}

// Solveur de Riemann HLL
State hll_flux(const State &WL, const State &WR) {
    State FL = flux(WL);
    State FR = flux(WR);

    double hL = max(WL.h, 0.0);
    double hR = max(WR.h, 0.0);
    double uL = (hL > 0.0) ? WL.hu / hL : 0.0;
    double uR = (hR > 0.0) ? WR.hu / hR : 0.0;
    double cL = sqrt(g * hL);
    double cR = sqrt(g * hR);

    double sL = min(uL - cL, uR - cR);
    double sR = max(uL + cL, uR + cR);

    State FHLL;
    if (sL >= 0.0) return FL;
    else if (sR <= 0.0) return FR;
    else {
        double denom = sR - sL;
        FHLL.h  = (sR * FL.h  - sL * FR.h  + sR * sL * (WR.h  - WL.h))  / denom;
        FHLL.hu = (sR * FL.hu - sL * FR.hu + sR * sL * (WR.hu - WL.hu)) / denom;
        return FHLL;
    }
}

// conditions périodiques
int idx_periodic(int i, int nx) {
    if (i < 0) return i + nx;
    if (i >= nx) return i - nx;
    return i;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Params p;
    int nx = p.nx;
    double dx = (p.xR - p.xL) / nx;

    vector<double> x(nx);
    for (int i = 0; i < nx; ++i)
        x[i] = p.xL + (i + 0.5) * dx;

    vector<State> W(nx), Wnew(nx);

    // Condition initiale : dam-break
    double hL = 2.0, hR = 1.0;
    double huL = 0.0, huR = 0.0;
    double x_mid = 0.5 * (p.xL + p.xR);
    for (int i = 0; i < nx; ++i) {
        if (x[i] < x_mid)
            W[i] = {hL, huL};
        else
            W[i] = {hR, huR};
    }

    // Fichier de sortie compatible Gnuplot
    ofstream fout("solution_godunov.dat");
    if (!fout) {
        cerr << "Erreur : impossible d'ouvrir le fichier de sortie.\n";
        return 1;
    }

    auto write_snapshot = [&](double time) {
    fout << "# time=" << time << "\n";
    for (int i = 0; i < nx; ++i) {
        double u = (W[i].h > p.hmin) ? W[i].hu / W[i].h : 0.0;
        fout << x[i] << " " << W[i].h << " " << u << "\n";
    }
    fout << "\n";  // ← Deux lignes vides séparent les blocs dans Gnuplot
};

    double t = 0.0;
    int step = 0;
    write_snapshot(t);

    // Boucle en temps
    while (t < p.tFinal) {
        double maxs = 1e-12;
        for (int i = 0; i < nx; ++i)
            maxs = max(maxs, max_wave_speed(W[i]));

        double dt = p.CFL * dx / maxs;
        if (t + dt > p.tFinal) dt = p.tFinal - t;

        // flux aux interfaces
        vector<State> Fhalf(nx);
        for (int i = 0; i < nx; ++i) {
            int ip = idx_periodic(i + 1, nx);
            Fhalf[i] = hll_flux(W[i], W[ip]);
        }

        // mise à jour conservative
        for (int i = 0; i < nx; ++i) {
            int im = idx_periodic(i - 1, nx);
            Wnew[i].h  = W[i].h  - (dt / dx) * (Fhalf[i].h  - Fhalf[im].h);
            Wnew[i].hu = W[i].hu - (dt / dx) * (Fhalf[i].hu - Fhalf[im].hu);
            if (Wnew[i].h < p.hmin) { Wnew[i].h = p.hmin; Wnew[i].hu = 0.0; }
        }

        W.swap(Wnew);
        t += dt;
        ++step;

        // écriture périodique pour visualisation
        if (step % 50 == 0 || t >= p.tFinal - 1e-12) {
            cout << "step " << step << " t=" << t << "\n";
            write_snapshot(t);
        }
    }

    fout.close();
    cout << "✅ Simulation terminée. Résultats dans solution_godunov.dat\n";
    return 0;
}
