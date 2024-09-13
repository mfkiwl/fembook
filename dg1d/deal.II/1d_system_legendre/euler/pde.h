//------------------------------------------------------------------------------
// 1d compressible Euler equations
//------------------------------------------------------------------------------

using namespace dealii;

// Number of PDE in the system
const unsigned int nvar = 3;

//------------------------------------------------------------------------------
// Extend namespace Problem with data specific to this pde
// This data must be set in problem_data.h file.
//------------------------------------------------------------------------------
namespace Problem
{
   extern double gamma;
}

//------------------------------------------------------------------------------
// Linear acoustics
//------------------------------------------------------------------------------
namespace PDE
{

   const double gamma = Problem::gamma;

// Numerical flux functions
   enum class FluxType {roe, rusanov};

   std::map<std::string, FluxType> FluxTypeList{{"roe",     FluxType::roe},
                                                {"rusanov", FluxType::rusanov}};

//------------------------------------------------------------------------------
// Flux of the PDE model: f(u)
//------------------------------------------------------------------------------
   void
   physical_flux(const Vector<double>& u,
                 const Point<1>&       /*p*/,
                 Vector<double>&       flux)
   {
      const double rho = u[0];
      const double vel = u[1] / rho;
      const double pre = (gamma - 1.0) * (u[2] - 0.5 * rho * pow(vel, 2));
      flux[0] = rho * vel;
      flux[1] = pre + rho * pow(vel, 2);
      flux[2] = (u[2] + pre) * vel;
   }

//------------------------------------------------------------------------------
// Maximum wave speed: |df/du(u)|
//------------------------------------------------------------------------------
   double
   max_speed(const Vector<double>& u,
             const Point<1>&       /*p*/)
   {
      const double rho = u[0];
      const double vel = u[1] / rho;
      const double pre = (gamma - 1.0) * (u[2] - 0.5 * rho * pow(vel, 2));
      return abs(vel) + sqrt(gamma * pre / rho);
   }

//------------------------------------------------------------------------------
// R = matrix of right eigenvectors, columns are right eigenvectors
// L = matrix of left eigenvectors = R^(-1), rows are left eigenvectors
//------------------------------------------------------------------------------
   void
   char_mat(const Vector<double>& /*u*/,
            const Point<1>&       /*p*/,
            FullMatrix<double>&   R,
            FullMatrix<double>&   L)
   {
      R(0, 0) = 1.0; R(0, 1) = 0.0; R(0, 2) = 0.0;
      R(1, 0) = 0.0; R(1, 1) = 1.0; R(1, 2) = 0.0;
      R(2, 0) = 0.0; R(2, 1) = 0.0; R(2, 2) = 1.0;

      L(0, 0) = 1.0; L(0, 1) = 0.0; L(0, 2) = 0.0;
      L(1, 0) = 0.0; L(1, 1) = 1.0; L(1, 2) = 0.0;
      L(2, 0) = 0.0; L(2, 1) = 0.0; L(2, 2) = 1.0;
   }

//------------------------------------------------------------------------------
// Compute flux across cell faces
//------------------------------------------------------------------------------
   void
   roe_flux(const Vector<double>& /*ul*/,
            const Vector<double>& /*ur*/,
            const Point<1>&       /*p*/,
            Vector<double>&       /*flux*/)
   {
      AssertThrow(false, ExcNotImplemented());
   }

//------------------------------------------------------------------------------
// Compute flux across cell faces
//------------------------------------------------------------------------------
   void
   rusanov_flux(const Vector<double>& ul,
                const Vector<double>& ur,
                const Point<1>&       p,
                Vector<double>&       flux)
   {
      Vector<double> fl(nvar);
      physical_flux(ul, p, fl);

      Vector<double> fr(nvar);
      physical_flux(ur, p, fr);

      double cl = max_speed(ul, p);
      double cr = max_speed(ur, p);
      double lam = std::max(cl, cr);

      for(unsigned int i=0; i<nvar; ++i)
         flux[i] = 0.5 * (fl[i] + fr[i]) - 0.5 * lam * (ur[i] - ul[i]);
   }

//------------------------------------------------------------------------------
// Compute flux across cell faces
//------------------------------------------------------------------------------
   void
   numerical_flux(const FluxType        flux_type,
                  const Vector<double>& ul,
                  const Vector<double>& ur,
                  const Point<1>&       p,
                  Vector<double>&       flux)
   {
      switch(flux_type)
      {
         case FluxType::roe:
            roe_flux(ul, ur, p, flux);
            break;

         case FluxType::rusanov:
            rusanov_flux(ul, ur, p, flux);
            break;

         default:
            AssertThrow(false, ExcMessage("Unknown flux type"));
      }
   }

}