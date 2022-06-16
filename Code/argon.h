#ifndef ARGON_H
#define ARGON_H
#include <random>
#include <fstream>
#include <tuple>
typedef unsigned short int usint;
typedef unsigned int uint;

class Argon
{
private:
    /// Declaration of parameters describing the system
    usint n;    ///< Number of atoms along the crystal edge
    usint So;   ///< Thermalisation steps
    uint Sd;    ///< Number of steps of core simulation
    usint Sout; ///< Save informations about the system every `Sout` steps
    usint Sxyz; ///< Save positions of atoms every `Sxyz` steps
    double m;   ///< Mass of the single atom
    double e;   ///< Minimum of the potential
    double R;   ///< Interatomic distance for which occurs minimum of the potential
    double k;   ///< Boltzmann constant
    double f;   ///< Elastic coefficient
    double L;   ///< Radius of sphere which confines atoms
    double a;   ///< Interatomic distance
    double T0;  ///< Initial temperature
    double tau; ///< Integration step

    /// Declaration of internal parameters
    usint N; ///< Total number of atoms (this especially denotes number of rows in the position and momentum arrays)
    usint K; ///< Dimension (this especially denotes number of columns in the position and momentum arrays)

    /// Declaration of bufors
    double *b0; ///< First egde (length) of elementary crystal cell
    double *b1; ///< Second egde (width) of elementary crystal cell
    double *b2; ///< Third egde (depth) of elementary crystal cell

    double *p;  ///< 1D array to store sum of momentum
    double *Vs; ///< 1D array to store trapping potentials

    double **r0; ///< 2D array to store atoms positions
    double **p0; ///< 2D array to store atoms momentum
    double **Vp; ///< 2D array to store van der Waals interactions
    double **Fs; ///< 2D array to store repulsion from sphere walls
    double **Fi; ///< 2D array to store total forces impact to atoms

    double ***Fp; ///< 3D array to store interaction forces between atoms

    bool initialPosMoCheck; ///< Indicates if initial state (positions and momenta) is calculated
    bool initialFoPoCheck;  ///< Indicates if initial state (forces and potentials) is calculated
    bool simulationRead;    ///< Indicates if simulation is ready to run
    std::mt19937 mt;        ///< High definition pseudo-random number generator

    // Physical parameters related to system
    double V;  ///< Total potential energy;
    double H;  ///< Hamiltonian at a given moment in time (total energy of the system - must be constant)
    double T;  ///< Temperature of the system at a given moment in time
    double P;  ///< Pressure of the system at a given moment in time
    double Ek; ///< Kinetic energy at a given moment in time

    // Mean values of physical parameters
    double Hmean; ///< Mean Hamiltonian
    double Tmean; ///< Mean Temperature
    double Pmean; ///< Mean Pressure

    bool fileIsEmpty(std::ifstream &input) const;
    void prepareFiles();
    void closeFiles();
    void calculateCurrentHTP();
    void saveCurrentHTP(const double &time);
    void saveCurrentPositions();
    void saveMeanHTP(const double &H, const double &T, const double &P);
    void printCurrentInfo(const double &time) const;

    std::ofstream ofileHTP;
    std::ofstream ofileMeanHTP;
    std::ofstream ofileRt;

public:
    Argon() noexcept;
    ~Argon() noexcept;

    void setParameters(const char *filename) noexcept(false);
    void checkParameters() const noexcept;
    void initialState();
    void saveInitialState(const char *rFilename, const char *pFilename) const;
    void initialForces();
    void simulationLoop();

    std::tuple<double *, usint> getMomentumAbs() const;
};

#endif // ARGON_H