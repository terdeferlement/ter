#include "SaintVenant.h"
#include <cmath>
#include <iostream>

using namespace std;

// ========================================
// Constructeur : initialise juste le temps à 0
// ========================================
SaintVenant1D::SaintVenant1D() : _t(0.0)
{
}

// ========================================
// Destructeur : ferme le fichier s'il est ouvert
// ========================================
SaintVenant1D::~SaintVenant1D()
{
    if (_fichier.is_open())
        _fichier.close();
}

// ========================================
// Initialiser la simulation
// N = nombre de cellules
// L = longueur du domaine
// CFL = nombre CFL (0.45 conseillé)
// nom_fichier = où sauvegarder les résultats
// ========================================
void SaintVenant1D::Initialiser(int N, double L, double CFL, string nom_fichier)
{
    _N = N;
    _L = L;
    _dx = L / N;  // Taille d'une cellule
    _CFL = CFL;
    _t = 0.0;
    
    // Allouer les vecteurs de taille N
    _h.resize(N);
    _hu.resize(N);
    _zb.resize(N); 
    _d_zb.resize(N);  
    
    // Ouvrir le fichier
    _fichier.open(nom_fichier);
    
    cout << "Simulation initialisée :" << endl;
    cout << "  - Nombre de cellules : " << N << endl;
    cout << "  - Longueur domaine : " << L << " m" << endl;
    cout << "  - Pas d'espace dx : " << _dx << " m" << endl;
}

void SaintVenant1D::ConditionInitialeDamBreak() {
    for (int i = 0; i < _N; i++) {
        if (i < _N / 2) _h[i] = 10.0; // Gauche haute
        else            _h[i] = 5.0; // Droite basse
        _hu[i] = 0.0;
    }
}

// ========================================
// Condition initiale : Soliton 
// Une vague unique qui se déplace vers la droite
// ========================================

void SaintVenant1D::ConditionInitialeSoliton(double A, double x_depart)
{
    double h0 = 2; // Profondeur au repos (doit correspondre à ton fond plat)
    _h_fond = h0;
    
    // 1. Célérité EXACTE du soliton (Vitesse de l'onde)
    // c = sqrt( g * (h0 + Amplitude) )
    // C'est la vitesse à laquelle la bosse avance
    double c = sqrt(_g * (h0 + A));
    
    // 2. Facteur de forme k (Largeur de la bosse)
    // Plus A est grand, plus la bosse est pointue
    double k = sqrt((3.0 * A) / (4.0 * pow(h0, 3)));

    cout << "Initialisation Soliton (Mode Physique) :" << endl;
    cout << "  - Amplitude : " << A << " m" << endl;
    cout << "  - Vitesse de l'onde (calculee) : " << c << " m/s" << endl;

    for (int i = 0; i < _N; i++)
    {
        double x = (i + 0.5) * _dx;
        
        // --- Calcul de la forme ---
        double arg = k * (x - x_depart);
        double sech = 1.0 / cosh(arg);
        double eta = A * sech * sech; // Perturbation de surface
        
        double H = h0 + eta;          // Hauteur totale (Surface)
        double h_reel = std::max(0.0, H - _zb[i]); // Hauteur d'eau (Surface - Fond)
        
        // --- Nettoyage (Anti-Crash) ---
        if (h_reel < critere_hauteur_deau) 
        {
            _h[i] = 0.0;
            _hu[i] = 0.0;
        }
        else
        {
            _h[i] = h_reel;
            
            // --- Vitesse du fluide (Physique) ---
            // La vitesse des particules d'eau n'est PAS la vitesse de l'onde c.
            // Relation : u = c * (eta / H)
            // Au sommet de la vague, l'eau avance. Loin de la vague, l'eau est immobile.
            double u_fluide = c * (eta / H);
            
            _hu[i] = _h[i] * u_fluide;
        }
    }
}

void SaintVenant1D::ConditionInitialeGaussienne(double amplitude, double position_x, double largeur, double vitesse_init)
{
    double niveau_moyen = 0.2; 
    _h_fond = niveau_moyen;
    
    cout << "Initialisation Gaussienne (Localisee) :" << endl;
    cout << "  - Amplitude : " << amplitude << " m" << endl;
    cout << "  - Vitesse   : " << vitesse_init << " m/s (Appliquee uniquement sous la bosse)" << endl;

    for (int i = 0; i < _N; i++)
    {
        double x = (i + 0.5) * _dx;
        
        // 1. Calcul de la forme de la gaussienne (entre 0 et 1)
        double dist = x - position_x;
        double facteur_forme = exp( - (dist * dist) / (largeur * largeur) );
        
        // 2. Hauteur d'eau
        double perturbation = amplitude * facteur_forme;
        double H = niveau_moyen + perturbation;
        double h_calc = std::max(0.0, H - _zb[i]);
        
        // 3. Application propre
        if (h_calc < critere_hauteur_deau) 
        {
            _h[i] = 0.0;
            _hu[i] = 0.0;
        } 
        else 
        {
            _h[i] = h_calc;
            
            // CORRECTION ICI : 
            // La vitesse est multipliée par le facteur de forme.
            // Si on est loin de la vague (facteur_forme ~ 0), la vitesse est nulle.
            // Si on est au sommet (facteur_forme = 1), la vitesse est 'vitesse_init'.
            
            double u_local = vitesse_init * facteur_forme;
            
            // Sécurité : Si l'amplitude est nulle (pas de vague), on ne met pas de vitesse du tout
            if (abs(amplitude) < 1e-9) u_local = 0.0;

            _hu[i] = _h[i] * u_local;
        }
    }
}
// Condition initiale de bathymetrie

void SaintVenant1D::DefinirFondPlat()
{
    // Remplir tout avec 0
    for (int i = 0; i < _N; i++)
    {
        _zb[i] = 0.0;
    }
    _h_fond = 1.0; // Valeur par défaut pour référence
    cout << "Bathymetrie : Fond plat (z=0)." << endl;
}

void SaintVenant1D::DefinirFondPente(double x_debut, double z_fin)
{
    // Pente linéaire à partir de x_debut
    double pente = z_fin / (_L - x_debut);
    
    for (int i = 0; i < _N; i++)
    {
        double x = (i + 0.5) * _dx;
        
        if (x < x_debut) 
        {
            _zb[i] = 0.0;
            _d_zb[i] =0.0;
        }
        else 
        {
            _zb[i] = pente * (x - x_debut);
            _d_zb[i] = pente;
        }
    }
    cout << "Bathymetrie : Pente démarrant a x=" << x_debut << "m." << endl;
}

void SaintVenant1D::DefinirFondMarche(double x_marche, double z_haut)
{
    for (int i = 0; i < _N; i++)
    {
        double x = (i + 0.5) * _dx;
        
        if (x < x_marche)
        {
            _zb[i] = 0.0;     // Partie basse (avant la marche)
        }
        else
        {
            _zb[i] = z_haut;  // Partie haute (sur la marche)
        }
    }
    cout << "Bathymetrie : Marche d'escalier a x=" << x_marche << "m (Hauteur=" << z_haut << "m)." << endl;
}


void SaintVenant1D::DefinirFondPentePuisPlat(double x_debut, double x_fin, double z_fin)
{
    // Sécurité anti-crash
    if (x_fin <= x_debut) {
        cout << "Erreur : x_fin doit etre plus grand que x_debut !" << endl;
        return;
    }

    double pente = z_fin / (x_fin - x_debut);
    
    cout << "Bathymetrie : Pente de x=" << x_debut << " a x=" << x_fin 
         << ", puis plateau a z=" << z_fin << "m." << endl;

    for (int i = 0; i < _N; i++)
    {
        double x = (i + 0.5) * _dx;
        
        if (x < x_debut)
        {
            // Zone 1 : Avant la pente (Fond plat profond)
            _zb[i] = 0.0;
            _d_zb[i] = 0.0;
        }
        else if (x > x_fin)
        {
            // Zone 3 : Après la pente (Plateau / Terre ferme)
            _zb[i] = z_fin;
            _d_zb[i] = 0.0;
        }
        else
        {
            // Zone 2 : Sur la pente
            _zb[i] = pente * (x - x_debut);
            _d_zb[i] = pente;
        }
    }
}




void SaintVenant1D::DefinirFondDoublePente(double x_debut, double x_cassure, double z_cassure, double z_fin)
{
    // Sécurités
    if (x_cassure <= x_debut || x_cassure >= _L) {
        cout << "Erreur Geometrie : Les points x doivent etre ordonnes (debut < cassure < L)" << endl;
        return;
    }

    // Calcul des deux pentes
    double pente_1 = z_cassure / (x_cassure - x_debut);
    double pente_2 = (z_fin - z_cassure) / (_L - x_cassure);
    
    cout << "Bathymetrie : Double Pente." << endl;
    cout << "  - Pente 1 (Douce) : de " << x_debut << " a " << x_cassure << "m (Pente=" << pente_1 << ")" << endl;
    cout << "  - Pente 2 (Raide) : de " << x_cassure << " a " << _L << "m (Pente=" << pente_2 << ")" << endl;

    for (int i = 0; i < _N; i++)
    {
        double x = (i + 0.5) * _dx;
        
        if (x < x_debut)
        {
            // Zone 1 : Fond plat profond
            _zb[i] = 0.0;
            _d_zb[i] = 0.0;
        }
        else if (x < x_cassure)
        {
            // Zone 2 : Première pente
            _zb[i] = pente_1 * (x - x_debut);
            _d_zb[i] = pente_1;
        }
        else
        {
            // Zone 3 : Deuxième pente
            // On repart de la hauteur z_cassure pour assurer la continuité
            _zb[i] = z_cassure + pente_2 * (x - x_cassure);
            _d_zb[i] = pente_2;
        }
    }
}







// ========================================
// Calculer le flux physique F(W)
// Pour Saint-Venant : F = (hu, hu²/h + g*h²/2)
// ========================================
void SaintVenant1D::CalculerFluxPhysique(double h, double hu, double& F_h, double& F_hu)
{
    // Première composante : F_h = hu
    F_h = hu;
    
    // Deuxième composante : F_hu = hu²/h + g*h²/2
    if (h > critere_hauteur_deau)  // Éviter division par zéro
    {
        double u = hu / h;
        F_hu = hu * u + 0.5 * _g * h * h;
    }
    else
    {
        F_hu = 0.0;
    }
}

// ========================================
// Calculer la vitesse u = hu/h
// ========================================
double SaintVenant1D::CalculerVitesse(double h, double hu)
{
    if (h > critere_hauteur_deau)
        return hu / h;
    else
        return 0.0;
}


// Remplace FluxRusanov par ceci  FluxHLL
void SaintVenant1D::FluxHLL(double hL, double huL, double hR, double huR,
                            double& flux_h, double& flux_hu)
{
    // 1. Calcul des vitesses et célérités
    double uL = (hL > 1e-8) ? huL / hL : 0.0;
    double uR = (hR > 1e-8) ? huR / hR : 0.0;
    
    double cL = sqrt(_g * hL);
    double cR = sqrt(_g * hR);
    
    // 2. Estimation des vitesses d'ondes (Approximation de Davis)
    // S_L = Vitesse de l'onde la plus à gauche
    // S_R = Vitesse de l'onde la plus à droite
    double S_L = std::min(uL - cL, uR - cR);
    double S_R = std::max(uL + cL, uR + cR);
    
    // 3. Calcul du flux selon la position de l'interface (x=0 local) par rapport aux ondes
    
    // Cas A : Supersonique à gauche (Tout part à droite)
    if (S_L >= 0.0) 
    {
        CalculerFluxPhysique(hL, huL, flux_h, flux_hu);
    }
    // Cas B : Supersonique à droite (Tout part à gauche)
    else if (S_R <= 0.0) 
    {
        CalculerFluxPhysique(hR, huR, flux_h, flux_hu);
    }
    // Cas C : Subsonique (On est dans l'éventail entre les deux ondes)
    else 
    {
        double F_L_h, F_L_hu;
        double F_R_h, F_R_hu;
        CalculerFluxPhysique(hL, huL, F_L_h, F_L_hu);
        CalculerFluxPhysique(hR, huR, F_R_h, F_R_hu);
        
        // Formule HLL
        double denom = S_R - S_L;
        flux_h  = (S_R * F_L_h  - S_L * F_R_h  + S_L * S_R * (hR - hL)) / denom;
        flux_hu = (S_R * F_L_hu - S_L * F_R_hu + S_L * S_R * (huR - huL)) / denom;
    }
}


// ========================================
// C'est la moyenne des flux + un terme de dissipation
// ========================================


void SaintVenant1D::FluxRusanov(double hL, double huL, double hR, double huR,
                                       double& flux_h, double& flux_hu)
{
    // 1. Calculer les flux physiques à gauche et à droite
    double FL_h, FL_hu;   // Flux gauche
    double FR_h, FR_hu;   // Flux droite
    
    CalculerFluxPhysique(hL, huL, FL_h, FL_hu);
    CalculerFluxPhysique(hR, huR, FR_h, FR_hu);
    
    // 2. Calculer les vitesses de propagation
    double uL = CalculerVitesse(hL, huL);
    double uR = CalculerVitesse(hR, huR);
    
    double cL = 0.0, cR = 0.0;
    if (hL > critere_hauteur_deau) cL = sqrt(_g * hL);  // Célérité gauche
    if (hR > critere_hauteur_deau) cR = sqrt(_g * hR);  // Célérité droite
    
    double lambdaL = abs(uL) + cL;  // Vitesse max à gauche
    double lambdaR = abs(uR) + cR;  // Vitesse max à droite
    double lambda = max(lambdaL, lambdaR);  // On prend le maximum
    
    // 3. Flux de Rusanov = moyenne + dissipation
    flux_h = 0.5 * (FL_h + FR_h) - 0.5 * lambda * (hR - hL);
    flux_hu = 0.5 * (FL_hu + FR_hu) - 0.5 * lambda * (huR - huL);
}

// ========================================
// Calculer la vitesse maximale dans le domaine
// Sert pour la condition CFL
// ========================================
double SaintVenant1D::VitesseMaximale()
{
    double v_max = 0.0;
    
    for (int i = 0; i < _N; i++)
    {
        double u = CalculerVitesse(_h[i], _hu[i]);
        double c = 0.0;
        if (_h[i] > 1e-10) 
            c = sqrt(_g * _h[i]);
        
        double vitesse = fabs(u) + c;
        v_max = max(v_max, vitesse);
    }
    
    return v_max;
}

// ========================================
// Calculer le pas de temps avec CFL
// dt = CFL * dx / vitesse_max
// ========================================
void SaintVenant1D::CalculerPasDeTemps()
{
    double v_max = VitesseMaximale();
    
    if (v_max > critere_vitesse)
        _dt = _CFL * _dx / v_max;
    else
        _dt = 0.01;  // Valeur par défaut si v_max = 0
}

// ========================================
// Avancer d'un pas de temps
// Schéma de Godunov avec flux de rosunov
// =======================================


void SaintVenant1D::Avancer()
{
    //  Calculer le nouveau pas de temps
    CalculerPasDeTemps();
    
    //  Créer des vecteurs temporaires pour la nouvelle solution
    vector<double> h_nouveau(_N);
    vector<double> hu_nouveau(_N);
    double coeff = _dt / _dx;
    
    //  Pour chaque cellule (sauf les bords)
    for (int i = 1; i < _N - 1; i++)
    {
        // ==========================================================
        //  NOUVELLE LOGIQUE : RECONSTRUCTION HYDROSTATIQUE (WELL-BALANCED)
        // ==========================================================
        
        // 1. Interface GAUCHE (entre i-1 et i)
        // ------------------------------------
        // On prend le "plus haut" fond à l'interface
        double z_inter_G = max(_zb[i-1], _zb[i]);
        
        // On reconstruit les hauteurs d'eau pour qu'elles "voient" la marche
        double h_G_L = max(0.0, _h[i-1] + _zb[i-1] - z_inter_G); // Gauche de l'interface
        double h_G_R = max(0.0, _h[i]   + _zb[i]   - z_inter_G); // Droite de l'interface
        
        // Calcul du flux GAUCHE avec les hauteurs reconstruites
        double flux_gauche_h, flux_gauche_hu;
        FluxHLL(h_G_L, _hu[i-1], h_G_R, _hu[i], 
                    flux_gauche_h, flux_gauche_hu);

        // 2. Interface DROITE (entre i et i+1)
        // ------------------------------------
        double z_inter_D = max(_zb[i], _zb[i+1]);
        
        double h_D_L = max(0.0, _h[i]   + _zb[i]   - z_inter_D); // Gauche de l'interface
        double h_D_R = max(0.0, _h[i+1] + _zb[i+1] - z_inter_D); // Droite de l'interface
        
        // Calcul du flux DROITE avec les hauteurs reconstruites
        double flux_droite_h, flux_droite_hu;
        FluxHLL(h_D_L, _hu[i], h_D_R, _hu[i+1], 
                    flux_droite_h, flux_droite_hu);


        // 3. Calcul du TERME SOURCE (Equilibre Hydrostatique)
        // ------------------------------------
        // Ce terme remplace votre ancien calcul "dzb_dx".
        // Il est calculé précisément pour annuler l'erreur du flux quand l'eau est au repos.
        // Formule : g/2 * (h_reconstruit^2 - h_reel^2)
        
        double TermeSource_G = 0.5 * _g * (pow(h_G_R, 2) - pow(_h[i], 2));
        double TermeSource_D = 0.5 * _g * (pow(h_D_L, 2) - pow(_h[i], 2));
        
        // Somme des sources (c'est un terme de force, pas un flux)
        double Source_WellBalanced = TermeSource_G + TermeSource_D;


        // 4. MISE A JOUR
        // ------------------------------------
        
        // Conservation de la masse (inchangé, sauf que les flux utilisent h reconstruit)
        h_nouveau[i] = _h[i] - coeff * (flux_droite_h - flux_gauche_h);
        
        // Conservation de la quantité de mouvement
        // On ajoute le terme source ici. Attention aux parenthèses.
        // Note: Le terme source ici est déjà "intégré", on le multiplie par coeff (dt/dx) 
        // car il agit comme une correction de flux aux interfaces.
        hu_nouveau[i] = _hu[i] - coeff * (flux_droite_hu - flux_gauche_hu) 
                               + coeff * Source_WellBalanced;

        // ==========================================================
        //  ANCIEN CODE (COMMENTÉ POUR MÉMOIRE)
        // ==========================================================
        /*
        // Flux à l'interface droite (entre i et i+1)
        double flux_droite_h_OLD, flux_droite_hu_OLD;
        FluxRusanov(_h[i], _hu[i], _h[i+1], _hu[i+1],
                         flux_droite_h_OLD, flux_droite_hu_OLD);
        
        // Flux à l'interface gauche (entre i-1 et i)
        double flux_gauche_h_OLD, flux_gauche_hu_OLD;
        FluxRusanov(_h[i-1], _hu[i-1], _h[i], _hu[i],
                         flux_gauche_h_OLD, flux_gauche_hu_OLD);
        
        // Mise à jour conservative :
        // W_nouveau = W_ancien - (dt/dx) * (Flux_droite - Flux_gauche)
        h_nouveau[i] = _h[i] - coeff * (flux_droite_h_OLD - flux_gauche_h_OLD);
        hu_nouveau[i] = _hu[i] - coeff * (flux_droite_hu_OLD - flux_gauche_hu_OLD);

        // Pente locale dz/dx (différence centrée)
        if (_h[i] > critere_hauteur_deau)
        {
            double dzb_dx = (_zb[i+1] - _zb[i-1]) / (2.0 * _dx);
            double Source = - _g * _h[i] * dzb_dx;
            hu_nouveau[i] += _dt * Source; // On ajoute dt * Source
        }
        */
    }
    
    //Conditions limite fenetre ouverte
    // Bord Gauche
    h_nouveau[0] = h_nouveau[1];
    hu_nouveau[0] = hu_nouveau[1];
    // Bord Droit
    double H_voisin = h_nouveau[_N-2] + _zb[_N-2];
    h_nouveau[_N-1] = max(0.0, H_voisin - _zb[_N-1]);
    // h_nouveau[_N-1] = h_nouveau[_N-2];
    hu_nouveau[_N-1] = hu_nouveau[_N-2];

    
    for (int i = 0; i < _N; i++) 
    {
        if (h_nouveau[i] < critere_hauteur_deau) 
        {
            h_nouveau[i] = 0.0;  // Hauteur nulle
            hu_nouveau[i] = 0.0; // Vitesse nulle (tue les vitesses fantômes)
        }
    }
    

    // //  Conditions aux limites : réflexion
    // h_nouveau[0] = h_nouveau[1];
    // hu_nouveau[0] = -hu_nouveau[1];
    // h_nouveau[_N-1] = h_nouveau[_N-2];
    // hu_nouveau[_N-1] = -hu_nouveau[_N-2];

    
    //  Copier la nouvelle solution
    _h = h_nouveau;
    _hu = hu_nouveau;
    
    //  Avancer le temps
    _t += _dt;
}

// ========================================
// Sauvegarder la solution dans le fichier
// Format : temps x h u
// ========================================
void SaintVenant1D::Sauvegarder()
{
    for (int i = 0; i < _N; i++)
    {
        double x = (i + 0.5) * _dx;
        double u = CalculerVitesse(_h[i], _hu[i]);
        double zb = _zb[i];
        double H = _h[i] + zb; // Surface libre (Niveau de l'eau)
        
        // On écrit : t x h u zb H
        _fichier << _t << " " << x << " " << _h[i] << " " << u << " " << zb << " " << H << endl;
    }
    _fichier << endl;
}






















// Validation


double SaintVenant1D::CalculerMasseTotale()
{
    double volume_total = 0.0;
    
    // On somme la hauteur d'eau de toutes les cellules
    for (int i = 0; i < _N; i++)
    {
        volume_total += _h[i];
    }
    
    // Volume = Somme des hauteurs * largeur d'une cellule
    return volume_total * _dx;
}


double SaintVenant1D::ObtenirSurfaceMax()
{
    // On cherche l'altitude maximale atteinte par l'eau (H = h + zb)
    // On initialise très bas
    double H_max = -99999.0; 
    
    for (int i = 0; i < _N; i++)
    {
        // On ne regarde que s'il y a un peu d'eau (pour éviter les bugs sur sol sec)
        if (_h[i] > 1e-6) 
        {
            double H_actuel = _h[i] + _zb[i];
            if (H_actuel > H_max) 
            {
                H_max = H_actuel;
            }
        }
    }
    return H_max;
}

double SaintVenant1D::ObtenirPositionCrete()
{
    double H_max = -99999.0;
    int i_max = 0;
    
    // On cherche l'altitude MAXIMALE de la surface (H)
    for (int i = 0; i < _N; i++)
    {
        double H_actuel = _h[i] + _zb[i];
        
        // On ne considère que les zones mouillées
        if (_h[i] > 1e-4 && H_actuel > H_max)
        {
            H_max = H_actuel;
            i_max = i;
        }
    }
    
    return (i_max + 0.5) * _dx;
}

double SaintVenant1D::CalculerEnergieTotale()
{
    double energie_totale = 0.0;
    
    for (int i = 0; i < _N; i++)
    {
        // 1. Energie Potentielle : 1/2 * g * h^2
        double Ep = 0.5 * _g * _h[i] * _h[i];
        
        // 2. Energie Cinétique : 1/2 * h * u^2  (ou 1/2 * (hu)^2 / h)
        double Ec = 0.0;
        if (_h[i] > 1e-10) {
            double u = _hu[i] / _h[i];
            Ec = 0.5 * _h[i] * u * u;
        }
        
        energie_totale += (Ep + Ec);
    }
    
    return energie_totale * _dx;
}