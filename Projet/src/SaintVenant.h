#ifndef _SAINT_VENANT_H
#define _SAINT_VENANT_H

#include <vector>
#include <string>
#include <fstream>

// ========================================
// Classe principale : résout Saint-Venant 1D
// ========================================
class SaintVenant1D
{
private:
    // Paramètres du domaine
    int _N;              // Nombre de cellules
    double _L;           // Longueur du domaine [0, L]
    double _dx;          // Pas d'espace : dx = L/N
    double critere_hauteur_deau=1e-4;
    double critere_vitesse=1e-10;
    
    // Paramètres temporels
    double _t;           // Temps actuel
    double _dt;          // Pas de temps
    double _CFL;         // Nombre CFL (< 0.5 pour stabilité)

    //Paramètres condition initiale
    double _h_fond;

    //Bathymetrie
    std::vector<double> _zb;  // Bathymétrie (altitude du fond)
    std::vector<double> _d_zb;  // Bathymétrie (pente du fond)



    // Variables de la solution
    // W = (h, hu) où h = hauteur, hu = débit
    std::vector<double> _h;   // Hauteur d'eau dans chaque cellule
    std::vector<double> _hu;  // Débit (h*u) dans chaque cellule
    
    // Constante physique
    static constexpr double _g = 9.81;  // Gravité (m/s²)
    
    // Fichier pour sauvegarder
    std::ofstream _fichier;

public:
    // Constructeur
    SaintVenant1D();
    
    // Destructeur
    ~SaintVenant1D();
    
    // Initialiser la simulation
    void Initialiser(int N, double L, double CFL, std::string nom_fichier);
    
    // Définir la condition initiale
    // Dans la section public
    void ConditionInitialeSoliton(double A, double x_depart);
    void ConditionInitialeDamBreak();

    void ConditionInitialeGaussienne(double amplitude, double position_x, double largeur, double vitesse_init);
    // Configuration de la géométrie (Bathymétrie)
    void DefinirFondPlat();
    void DefinirFondPente(double x_debut, double z_fin);
    void DefinirFondMarche(double x_marche, double z_haut);
    void DefinirFondPentePuisPlat(double x_debut, double x_fin, double z_fin);
    void DefinirFondDoublePente(double x_debut, double x_cassure, double z_cassure, double z_fin);



    // Calculer le flux physique F(h, hu) = (hu, hu²/h + g*h²/2)
    void CalculerFluxPhysique(double h, double hu, double& F_h, double& F_hu);
    
    // Calculer le flux numérique de Lax-Friedrichs entre deux cellules
    void FluxRusanov(double hL, double huL, double hR, double huR,
                           double& flux_h, double& flux_hu);

    void FluxHLL(double hL, double huL, double hR, double huR,
                            double& flux_h, double& flux_hu);
    
    // Calculer la vitesse u = hu/h
    double CalculerVitesse(double h, double hu);
    
    // Calculer la vitesse maximale (pour le pas de temps CFL)
    double VitesseMaximale();
    
    // Calculer le pas de temps avec la condition CFL
    void CalculerPasDeTemps();
    
    // Avancer d'un pas de temps (schéma de Godunov)
    void Avancer();
    
    // Sauvegarder la solution dans le fichier
    void Sauvegarder();
    
    // Pour valider la quantité de masse
    double CalculerMasseTotale();
    // Pour valider la conservation de la vitesse
    double ObtenirPositionCrete(); // Retourne le X où h est maximal
    double ObtenirHauteurMax();    // Retourne l'épaisseur d'eau max (h)
    double ObtenirSurfaceMax();    // Retourne l'altitude max (h + zb) 
    // Pour valider l'energie
    double CalculerEnergieTotale();
    // Accesseurs
    double ObtenirTemps() const { return _t; }
    double ObtenirDt() const { return _dt; }
    double ObtenirHFond() const { return _h_fond; }
    const std::vector<double>& ObtenirZb() const { return _zb; }
    const std::vector<double>& ObtenirdZb() const { return _d_zb; }


    // Accesseurs pour debug
double ObtenirH(int i) const { 
    if (i >= 0 && i < _N) return _h[i]; 
    return 0.0; 
}
double ObtenirZb(int i) const { 
    if (i >= 0 && i < _N) return _zb[i]; 
    return 0.0; 
}
int ObtenirN() const { return _N; }
};

#endif // _SAINT_VENANT_H