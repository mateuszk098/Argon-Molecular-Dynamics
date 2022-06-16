#define _USE_MATH_DEFINES
#include "argon.h"
#include <cmath>
#include <ctime>
#include <iostream>
#include <string>
#include <iomanip>

/**
 * Checks if the input file is empty.
 **/
inline bool Argon::fileIsEmpty(std::ifstream &input) const
{
    return input.peek() == std::ifstream::traits_type::eof();
}

/**
 * This function opens and prepares files to write information about system
 * and current positions of atoms.
 **/
void Argon::prepareFiles()
{
    ofileHTP.open("../Out/HTP.txt", std::ios::out);
    ofileHTP << std::fixed << std::setprecision(5);
    ofileHTP << "t (ps)\t";
    ofileHTP << "H (kJ/mol)\t";
    ofileHTP << "T (K)\t";
    ofileHTP << "P (atm)\n";

    ofileRt.open("../Out/rt_data.txt", std::ios::out);
    ofileRt << std::fixed << std::setprecision(3);
}

/**
 * Closes files opened by `prepareFiles()`.
 **/
void Argon::closeFiles()
{
    ofileHTP.close();
    ofileRt.close();
}

/**
 * This function calculates current Hamiltonian, Temperature and Pressure
 * of the system. It uses current momenta and sphere repulsion to this.
 **/
void Argon::calculateCurrentHTP()
{
    // Prepare to accumulate physical parameters at initial time
    H = V; // At this moment Hamiltonian is just total potential
    T = 0.;
    P = 0.;
    Ek = 0.;

    for (usint i = 0; i < N; i++)
    {
        // Local variable to increase performance;
        Ek = (p0[i][0] * p0[i][0] + p0[i][1] * p0[i][1] + p0[i][2] * p0[i][2]) / (2. * m);

        // Accumulate physical parameters
        H += Ek;
        T += 2. / (3. * N * k) * Ek;
        P += sqrt(Fs[i][0] * Fs[i][0] + Fs[i][1] * Fs[i][1] + Fs[i][2] * Fs[i][2]) / (4. * M_PI * L * L);
    }
}

/**
 * Writes to file current time, Hamiltonian, Temperature and Pressure of the system.
 **/
inline void Argon::saveCurrentHTP(const double &time)
{
    ofileHTP << time << '\t' << H << '\t' << T << '\t' << P << '\n';
}

/**
 * Writes to file current positions of the atoms.
 **/
void Argon::saveCurrentPositions()
{
    // Number of atoms to read by Jmol
    ofileRt << N;
    ofileRt << "\n\n";

    for (usint i = 0; i < N; i++)
    {
        ofileRt << "AR ";

        for (usint j = 0; j < K; j++)
            ofileRt << r0[i][j] << ' ';
        ofileRt << '\n';
    }

    ofileRt << '\n';
}

/**
 * Writes to file mean values of Hamiltonian, Temperature and Pressure.
 * @param double Mean Hamiltonian,
 * @param double Mean Temperature,
 * @param double Mean Pressure.
 **/
void Argon::saveMeanHTP(const double &H, const double &T, const double &P)
{
    ofileMeanHTP.open("../Out/HTP-MEAN.txt", std::ios::out);
    ofileMeanHTP << std::fixed << std::setprecision(5);
    ofileMeanHTP << "H (kJ/mol)\t";
    ofileMeanHTP << "T (K)\t";
    ofileMeanHTP << "P (atm)\n";
    ofileMeanHTP << H << '\t' << T << '\t' << P << '\n';
    ofileMeanHTP.close();
}

/**
 * Print current information about the system while simulation is in progress.
 * @param double current time.
 **/
void Argon::printCurrentInfo(const double &time) const
{
    std::cout << std::fixed << std::setprecision(5);
    std::cout << "Current Time:             " << time << '\n';
    std::cout << "Current Total Energy:     " << H << '\n';
    std::cout << "Current Total Potential:  " << V << '\n';
    std::cout << "Current Temperature:      " << T << '\n';
    std::cout << "Current Pressure:         " << P << '\n';
    std::cout << '\n';
}

/**
 * Default constructor initializes exmaple parameters and memory to store informations about the system.
 **/
Argon::Argon() noexcept : n(7), So(100), Sd(10000), Sout(100), Sxyz(100), m(1.), e(1.), R(0.38), k(8.31e-3),
                          f(1e4), L(5.), a(0.38), T0(1e3), tau(1e-3), initialPosMoCheck(false),
                          initialFoPoCheck(false), mt(std::mt19937(time(nullptr)))
{
    N = n * n * n; // System is defined as 3D
    K = 3;

    std::cout << "`Argon()` says >: Initialized parameters to default values." << '\n';
    std::cout << "`Argon()` says >: Set pseudo-random number generator std::mt19937." << '\n';

    // Allocate memory and immediately set the values
    b0 = new double[K]{a, 0., 0.};
    b1 = new double[K]{a * 0.5, a * sqrt(3.) * 0.5, 0.};
    b2 = new double[K]{a * 0.5, a * sqrt(3.) / 6., a * sqrt(6.) / 3.};

    // Note the parenthesis at the end of new. These caused the allocated memory's
    // value to be set to zero (value-initialize)
    p = new double[K]();
    Vs = new double[N]();

    r0 = new double *[N]();
    p0 = new double *[N]();
    Vp = new double *[N]();
    Fs = new double *[N]();
    Fi = new double *[N]();

    Fp = new double **[N]();

    for (usint i = 0; i < N; i++)
    {
        r0[i] = new double[K]();
        p0[i] = new double[K]();
        Vp[i] = new double[N]();
        Fs[i] = new double[K]();
        Fi[i] = new double[K]();

        Fp[i] = new double *[N]();

        for (usint j = 0; j < N; j++)
            Fp[i][j] = new double[K]();
    }

    std::cout << "`Argon()` says >: Allocated memory for buffors.\n\n";
}

/**
 * Destructor frees up buffers memory.
 **/
Argon::~Argon() noexcept
{
    delete[] b0;
    delete[] b1;
    delete[] b2;

    delete[] p;
    delete[] Vs;

    for (usint i = 0; i < N; i++)
    {
        delete[] r0[i];
        delete[] p0[i];
        delete[] Vp[i];
        delete[] Fs[i];
        delete[] Fi[i];

        for (usint j = 0; j < N; j++)
            delete[] Fp[i][j];

        delete[] Fp[i];
    }

    delete[] r0;
    delete[] p0;
    delete[] Vp;
    delete[] Fs;
    delete[] Fi;

    delete[] Fp;

    std::cout << "`~Argon() says >: Memory released.\n\n";
}

/**
 * This function reads parameters from input file and sets appropriate variables.
 * Moreover it provides exception handling for invalid parameters and files.
 * @param char* filename with parameters to set.
 **/
void Argon::setParameters(const char *filename) noexcept(false)
{
    std::ifstream input;
    std::string tmp;

    try
    {
        input.open("../Config/" + std::string(filename), std::ios::in);

        if (input.fail())
            throw std::ifstream::failure("Exception opening/reading input file");

        if (fileIsEmpty(input))
            throw std::ifstream::failure("Exception input file is empty");

        input >>
            n >> tmp >> m >> tmp >> e >> tmp >> R >> tmp >> k >> tmp >> f >> tmp >> L >> tmp >> a >> tmp;
        input >> T0 >> tmp >> tau >> tmp >> So >> tmp >> Sd >> tmp >> Sout >> tmp >> Sxyz >> tmp;

        if (n < 1 || n > 25)
            throw std::invalid_argument("Invalid argument: n. Must be between 1 and 15.");
        if (m < 0.)
            throw std::invalid_argument("Invalid argument: m. Must be positive.");
        if (e < 0.)
            throw std::invalid_argument("Invalid argument: e. Must be positive.");
        if (R < 0.)
            throw std::invalid_argument("Invalid argument: R. Must be positive.");
        if (k < 0. || k > 1.)
            throw std::invalid_argument("Invalid argument: k. Must be between 0 and 1.");
        if (f < 0.)
            throw std::invalid_argument("Invalid argument: f. Must be positive.");
        if (L < 1.22 * (n - 1) * a)
            throw std::invalid_argument("Invalid argument: L. Must be greater than 1.22(n-1)a.");
        if (a < 0.)
            throw std::invalid_argument("Invalid argument: a. Must be positive.");
        if (T0 < 0.)
            throw std::invalid_argument("Invalid argument: T0. Must be positive.");
        if (tau < 0. || tau > 1e-2)
            throw std::invalid_argument("Invalid argument: tau. Must be between 0 and 1e-2.");
        if (So < 0 || So > Sd)
            throw std::invalid_argument("Invalid argument: So. Must be between between 0 and Sd.");
        if (Sd < 0)
            throw std::invalid_argument("Invalid argument: Sd. Must be positive.");
        if (Sout < 0 || Sout > Sd)
            throw std::invalid_argument("Invalid argument: Sout. Must be between between 0 and Sd.");
        if (Sxyz < 0 || Sxyz > Sd)
            throw std::invalid_argument("Invalid argument: Sxyz. Must be between between 0 and Sd.");

        std::cout << "`setParameters()` says >: Successfully set parameters from ../Config/" << filename << '\n';

        // N and K are still the same so I can carefully release the memory
        delete[] b0;
        delete[] b1;
        delete[] b2;

        delete[] p;
        delete[] Vs;

        for (usint i = 0; i < N; i++)
        {
            delete[] r0[i];
            delete[] p0[i];
            delete[] Vp[i];
            delete[] Fs[i];
            delete[] Fi[i];

            for (usint j = 0; j < N; j++)
                delete[] Fp[i][j];

            delete[] Fp[i];
        }

        delete[] r0;
        delete[] p0;
        delete[] Vp;
        delete[] Fs;
        delete[] Fi;

        delete[] Fp;

        // Here I can set the new values
        N = n * n * n;
        K = 3;

        b0 = new double[K]{a, 0., 0.};
        b1 = new double[K]{a * 0.5, a * sqrt(3.) * 0.5, 0.};
        b2 = new double[K]{a * 0.5, a * sqrt(3.) / 6., a * sqrt(6.) / 3.};

        p = new double[K]();
        Vs = new double[N]();

        r0 = new double *[N]();
        p0 = new double *[N]();
        Vp = new double *[N]();
        Fs = new double *[N]();
        Fi = new double *[N]();

        Fp = new double **[N]();

        for (usint i = 0; i < N; i++)
        {
            r0[i] = new double[K]();
            p0[i] = new double[K]();
            Vp[i] = new double[N]();
            Fs[i] = new double[K]();
            Fi[i] = new double[K]();

            Fp[i] = new double *[N]();

            for (usint j = 0; j < N; j++)
                Fp[i][j] = new double[K]();
        }

        input.close();
        std::cout << "`setParameters()` says >: Successfully reallocated memory for new parameters.\n\n";
    }
    catch (const std::invalid_argument &error)
    {
        // Notice I do not need to reallocate memory because of default parameters
        // and buffors have the same sizes
        n = 7;
        So = 100;
        Sd = 10000;
        Sout = 100;
        Sxyz = 100;
        m = 1.;
        R = 0.38;
        e = 1.;
        k = 8.31e-3;
        f = 1e4;
        L = 5.;
        a = 0.38;
        T0 = 1e3;
        tau = 1e-3;

        input.close();
        std::cerr << "`setParameters()` says >: Exception while setting parameters from ../Config/" << filename << '\n';
        std::cerr << "`setParameters()` says >: " << error.what() << '\n';
        std::cerr << "`setParameters()` says >: Values are set to default now.\n\n";
    }
    catch (const std::ifstream::failure &error)
    {
        n = 7;
        So = 100;
        Sd = 10000;
        Sout = 100;
        Sxyz = 100;
        m = 1.;
        R = 0.38;
        e = 1.;
        k = 8.31e-3;
        f = 1e4;
        L = 5.;
        a = 0.38;
        T0 = 1e3;
        tau = 1e-3;

        std::cerr << "`setParameters()` says >: " << error.what() << '\n';
        std::cerr << "`setParameters()` says >: Values are set to default now.\n\n";
    }
}

/**
 * This function prints all currently set parameters.
 **/
void Argon::checkParameters() const noexcept
{
    std::cout << "`checkParameters()` says >: Currently set parameters." << '\n';
    std::cout << "`checkParameters()` says >: n:        " << n << '\n';
    std::cout << "`checkParameters()` says >: m:        " << m << '\n';
    std::cout << "`checkParameters()` says >: e:        " << e << '\n';
    std::cout << "`checkParameters()` says >: R:        " << R << '\n';
    std::cout << "`checkParameters()` says >: k:        " << k << '\n';
    std::cout << "`checkParameters()` says >: f:        " << f << '\n';
    std::cout << "`checkParameters()` says >: L:        " << L << '\n';
    std::cout << "`checkParameters()` says >: a:        " << a << '\n';
    std::cout << "`checkParameters()` says >: T_0:      " << T0 << '\n';
    std::cout << "`checkParameters()` says >: tau:      " << tau << '\n';
    std::cout << "`checkParameters()` says >: So:       " << So << '\n';
    std::cout << "`checkParameters()` says >: Sd:       " << Sd << '\n';
    std::cout << "`checkParameters()` says >: S_out:    " << Sout << '\n';
    std::cout << "`checkParameters()` says >: Sxyz:     " << Sxyz << '\n';
    std::cout << "`checkParameters()` says >: End of parameters.\n\n";
}

/**
 * This function calculates initial positions and momenta of atoms.
 **/
void Argon::initialState()
{
    // Calculate initial positions of atoms (5)
    for (usint i_0 = 0; i_0 < n; i_0++)
    {
        for (usint i_1 = 0; i_1 < n; i_1++)
        {
            for (usint i_2 = 0; i_2 < n; i_2++)
            {
                for (usint j = 0; j < K; j++)
                {
                    usint i = i_0 + i_1 * n + i_2 * n * n;
                    r0[i][j] = (i_0 - 0.5 * (n - 1)) * b0[j] + (i_1 - 0.5 * (n - 1)) * b1[j] + (i_2 - 0.5 * (n - 1)) * b2[j];
                }
            }
        }
    }

    // Calculate initial momenta of atoms (7)
    for (usint i = 0; i < N; i++)
    {
        for (usint j = 0; j < K; j++)
        {
            // Random number between 0 and 1
            double number = static_cast<double>(mt()) / static_cast<double>(mt.max());
            // Random sign of momentum variable
            char sign = rand() % 2;

            // Calculate momentum
            if (sign == 0)
                p0[i][j] = -sqrt(-0.5 * k * T0 * log(number) * 2. * m); // (7)
            else if (sign == 1)
                p0[i][j] = +sqrt(-0.5 * k * T0 * log(number) * 2. * m); // (7)

            // Accumulate momenta
            p[j] += p0[i][j];
        }
    }

    // Eliminate the centre of mass movement (8)
    for (usint i = 0; i < N; i++)
    {
        for (usint j = 0; j < K; j++)
        {
            p0[i][j] = p0[i][j] - (p[j] / N);
        }
    }

    initialPosMoCheck = true;
    std::cout << "`initialState()` says >: Successfully calculated initial positions and momenta of atoms.\n\n";
}

/** This function saves initial state to given files.
 * @param char* filename where to save initial positions,
 * @param char* filename where to save initial momenta.
 **/
void Argon::saveInitialState(const char *rFilename, const char *pFilename) const
{
    std::ofstream rOut("../Out/" + std::string(rFilename), std::ios::out);
    std::ofstream pOut("../Out/" + std::string(pFilename), std::ios::out);

    // Number of atoms in header to read by Jmol
    rOut << N << "\n\n";
    pOut << N << "\n\n";

    for (usint i = 0; i < N; i++)
    {
        // AR beacuse of we analyse Argon gas
        rOut << "AR ";
        pOut << "AR ";

        for (usint j = 0; j < K; j++)
        {
            rOut << r0[i][j] << ' ';
            pOut << p0[i][j] << ' ';
        }

        rOut << '\n';
        pOut << '\n';
    }

    rOut.close();
    pOut.close();

    if (initialPosMoCheck == false)
    {
        std::cout << "`saveInitialState()` says >: Warning - did not calculate initial state!" << '\n';
        std::cout << "`saveInitialState()` says >: Warning - saved default values to ../Out/";
        std::cout << rFilename << " and ../Out/" << pFilename << "\n\n";
    }
    else
    {
        std::cout << "`saveInitialState()` says >: Successfully saved initial positions and momenta to ../Out/";
        std::cout << rFilename << " and ../Out/" << pFilename << "\n\n";
    }
}

/**
 * This function calculates initial forces and potentials acting on atoms in initial state.
 * Primarily we calculate there the trapping potentials, repulsion related to sphere walls,
 * total forces impact to particles, van der Waals interactions, interaction forces between atoms,
 * and total potential. Next that function calculates total energy (Hamiltonian), initial real
 * temperature (T) and initial pressure related to sphere walls.
 **/
void Argon::initialForces()
{
    // Total potential energy
    V = 0.;

    for (usint i = 0; i < N; i++)
    {
        // Absolute value of r_i -> |r_i|
        double r_i = sqrt(r0[i][0] * r0[i][0] + r0[i][1] * r0[i][1] + r0[i][2] * r0[i][2]);

        // (10)
        if (r_i < L)
            Vs[i] = 0.;
        else if (r_i >= L)
            Vs[i] = 0.5 * f * (r_i - L) * (r_i - L);

        // Accumulate potential related to sphere walls to total potential
        V += Vs[i];

        // (14)
        for (usint j = 0; j < K; j++)
        {
            if (r_i < L)
                Fs[i][j] = 0.;
            else if (r_i >= L)
                Fs[i][j] = f * (L - r_i) * r0[i][j] / r_i;

            // Accumulate repulsive forces related to sphere walls to total forces
            Fi[i][j] = Fs[i][j];
        }

        // (9) and (13)
        for (usint j = 0; j < i; j++)
        {
            // Absolute value of r_i - r_j -> |r_i - r_j|
            double r_ij = sqrt((r0[i][0] - r0[j][0]) * (r0[i][0] - r0[j][0]) + (r0[i][1] - r0[j][1]) * (r0[i][1] - r0[j][1]) +
                               (r0[i][2] - r0[j][2]) * (r0[i][2] - r0[j][2]));

            // Local variables to evaluate powers -> huge increase of performance (instead of calculate with common pow())
            double y = (R / r_ij) * (R / r_ij);
            double x = y * y * y;
            // (9)
            Vp[i][j] = e * x * (x - 2.);

            for (usint k = 0; k < K; k++)
            {
                // Fp is not required as 3D array, we may use just ordinary variable but with Fp is more evident
                // what is happens here
                Fp[i][j][k] = 12. * e * x * (x - 1.) * (r0[i][k] - r0[j][k]) / (r_ij * r_ij);

                // Symmetry of forces matrix (only one triangular matrix needs to be calculated) -> increase performance
                Fi[i][k] += Fp[i][j][k];
                Fi[j][k] -= Fp[i][j][k];
            }

            // Accumulate van der Waals potentials
            V += Vp[i][j];
        }
    }

    prepareFiles();
    calculateCurrentHTP();
    saveCurrentHTP(0.);

    // Initial forces and potentials should be calculated
    initialFoPoCheck = true;

    std::cout << "`initialForces()` says >: Successfully calculated initial forces and potentials.\n\n";
}

void Argon::simulationLoop()
{
    if (initialPosMoCheck == true && initialFoPoCheck == true)
    {
        std::cout << "`simulationLoop()` says >: System is ready to simulation.\n\n";

        // Save initial positions
        saveCurrentPositions();
        // Current information to track simulation
        printCurrentInfo(0.);

        Hmean = 0.;
        Tmean = 0.;
        Pmean = 0.;

        uint infoOut = Sd / 10;

        // Simulation loop
        for (uint s = 1; s <= So + Sd; s++)
        {
            if (s % infoOut == 0)
            {
                printCurrentInfo(s * tau);
            }

            // Calculate auxiliary momenta (18a) and positions (18b)
            for (usint i = 0; i < N; i++)
            {
                for (usint j = 0; j < K; j++)
                {
                    p0[i][j] = p0[i][j] + 0.5 * Fi[i][j] * tau;
                    r0[i][j] = r0[i][j] + p0[i][j] * tau / m;
                }
            }

            // In every step set total potential to zero (IMPORTANT!)
            V = 0.;

            // Dynamics loop
            for (usint i = 0; i < N; i++)
            {
                // Absolute value of r_i -> |r_i|
                double r_i = sqrt(r0[i][0] * r0[i][0] + r0[i][1] * r0[i][1] + r0[i][2] * r0[i][2]);

                // (10)
                if (r_i < L)
                    Vs[i] = 0.;
                else if (r_i >= L)
                    Vs[i] = 0.5 * f * (r_i - L) * (r_i - L);

                // Accumulate potential related to sphere walls to total potential
                V += Vs[i];

                // (14)
                for (usint j = 0; j < K; j++)
                {
                    if (r_i < L)
                        Fs[i][j] = 0.;
                    else if (r_i >= L)
                        Fs[i][j] = f * (L - r_i) * r0[i][j] / r_i;

                    // Accumulate repulsive forces related to sphere walls to total forces
                    Fi[i][j] = Fs[i][j];
                }

                for (usint j = 0; j < i; j++)
                {
                    // Absolute value of r_i - r_j -> |r_i - r_j|
                    double r_ij = sqrt((r0[i][0] - r0[j][0]) * (r0[i][0] - r0[j][0]) + (r0[i][1] - r0[j][1]) * (r0[i][1] - r0[j][1]) +
                                       (r0[i][2] - r0[j][2]) * (r0[i][2] - r0[j][2]));

                    // Local variables to evaluate powers -> huge increase of performance (instead of calculate with common pow())
                    double y = (R / r_ij) * (R / r_ij);
                    double x = y * y * y;
                    // (9)
                    Vp[i][j] = e * x * (x - 2);

                    for (usint k = 0; k < K; k++)
                    {
                        Fp[i][j][k] = 12. * e * x * (x - 1) * (r0[i][k] - r0[j][k]) / (r_ij * r_ij);

                        // Symmetry of forces matrix (only one triangular matrix needs to be calculated) -> increase performance
                        Fi[i][k] += Fp[i][j][k];
                        Fi[j][k] -= Fp[i][j][k];
                    }

                    // Accumulate van der Waals potentials
                    V += Vp[i][j];
                }
            }

            // Calculate momenta (18c)
            for (usint i = 0; i < N; i++)
            {
                for (usint j = 0; j < K; j++)
                {
                    p0[i][j] = p0[i][j] + 0.5 * Fi[i][j] * tau;
                }
            }

            // Save temporary positions at given time
            if (s % Sxyz == 0)
            {
                saveCurrentPositions();
            }

            // Calculate and save temporary H, T and P at given time
            if (s % Sout == 0)
            {
                calculateCurrentHTP();
                saveCurrentHTP(s * tau);
            }

            // Accumulate mean values
            if (s >= So)
            {
                Tmean += T;
                Pmean += P;
                Hmean += H;
            }
        }

        printCurrentInfo((So + Sd) * tau);
        saveMeanHTP(Hmean / Sd, Tmean / Sd, Pmean / Sd);
        closeFiles();
    }
    else
    {
        std::cerr << "`simulationLoop()` says >: Error - system is not ready to simulation!\n";
        std::cerr << "`simulationLoop()` says >: Error - calculate initial state before!\n";
        std::cerr << "`simulationLoop()` says >: Error - leave the simulation.\n\n";
    }
}

std::tuple<double *, usint> Argon::getMomentumAbs() const
{
    double *pAbs = new double[N];

    // Calculate absolute value of momentum for every particle
    for (usint i = 0; i < N; i++)
    {
        for (usint j = 0; j < K; j++)
        {
            pAbs[i] += p0[i][j] * p0[i][j];
        }
        pAbs[i] = sqrt(pAbs[i]);
    }

    return std::make_tuple(pAbs, N);
}