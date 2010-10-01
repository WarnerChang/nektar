/**
* \page pageIntroduction Introduction to the spectral/hp element method
 
* \section Background Background
 
 The spectral/hp element method combines the geometric flexibility of
 classical \a h-type finite element techniques with the desirable
 resolution properties of spectral methods. In this approach a
 polynomial expansion of order \a P is applied to every elemental domain
 of a coarse finite element type mesh.  These techniques have been
 applied in many fundamental studies of fluid mechanics
 (Sherwin & Karniadakis 1996) and more recently have gained greater popularity
 in the modelling of wave-based phenomena such as computational
 electromagnetics (Hesthaven & Warburton 2002) and shallow water problems
 (Bernard, Remacle, Combien, Legat & Hillewaert 2009) - particularly when applied within a
 Discontinuous Galerkin formulation.
 
 There are at least two major challenges which arise in developing an
 efficient implementation of a \spectralhp element discretisation:
 - implementing the mathematical structure of the technique in a
 digestible, generic and coherent manner, and
 - designing and implementing the numerical methods and data
 structures in a matter so that both high- and low-order
 discretisations can be efficiently applied.
 
 In order to design algorithms which are efficient for both low- and
 high-order spectral/hp discretisations, it is important clearly define
 what we mean with low- and high-order. The spectral/hp element method
 can be considered as bridging the gap between the high-order end of
 the traditional finite element method and low-order end of
 conventional spectral methods. However, the concept of high- and
 low-order discretisations can mean very different things to these
 different communities. For example, the seminal works by Zienkiewicz
 & Taylor (1989) and Hughes (1987) list
 examples of elemental expansions only up to third or possibly
 fourth-order, implying that these orders are considered to be
 high-order for the traditional \a h-type finite element community. In
 contrast the text books of the spectral/hp element community
 typically show examples of problems ranging from a low-order bound of
 minimally fourth-order up to anything ranging from \f$10^{th}\f$-order to
 \f$15^{th}\f$-order polynomial expansions. On the other end of the
 spectrum, practitioners of global (Fourier-based) spectral methods
 (Gottlieb & Orzag 1977) would probably consider a \f$16^{th}\f$-order global
 expansion to be relatively low-order approximation.
 
 One could wonder whether these different definitions of low- and
 high-order are just inherent to the tradition and lore of each of the
 communities or whether there are more practical reasons for this
 distinct interpretation. Proponents of lower-order methods might
 highlight that some problems of practical interest are so
 geometrically complex that one cannot computationally afford to use
 high-order techniques on the massive meshes required to capture the
 geometry. Alternatively, proponents of high-order methods highlight
 that if the problem of interest can be captured on a computational
 domain at reasonable cost then using high-order approximations for
 sufficiently \a smooth solutions will provide a higher accuracy for
 a given computational cost. If one however probes even further it also
 becomes evident that the different communities choose to implement
 their algorithms in different manners. For example the standard
 \a h-type finite element community will typically uses techniques such
 as sparse matrix storage formats (where only the non-zero entries of a
 global matrix are stored) to represent a global operator.  In contrast
 the spectral/hp element community acknowledges that for higher
 polynomial expansions more closely coupled computational work takes
 place at the individual elemental level and this leads to the use of
 elemental operators rather than global matrix operators. In addition
 the global spectral method community often make use of the
 tensor-product approximations where products of one-dimensional rules
 for integration and differentiation can be applied.
 
* \section Definition Methods overview
 
 Here a review of some terminology in order to situate the spectral/hp element method within
 the field of the finite element methods.
 
* \subsection FEM The finite element method (FEM)
 
 Nowadays, the finite element method is one of the most popular
 numerical methods in the field of both solid and fluid mechanics. It
 is a discretisation technique used to solve (a set of) partial
 differential equations in its equivalent variational form. The
 classical approach of the finite element method is to partition the
 computational domain into a mesh of many small subdomains and to
 approximate the unknown solution by piecewise linear interpolation
 functions, each with local support. The FEM has been widely discussed
 in literature and for a complete review of the method, the reader is
 also directed to the seminal work of Zienkiewicz and Taylor (1989).
 
* \subsection HOFEM High-order finite element methods
 
 While in the classical finite element method the solution is expanded
 in a series of linear basis functions, high-order FEMs employ
 higher-order polynomials to approximate the solution. For the
 high-order FEM, the solution is locally expanded into a set of \a P+1
 linearly independent polynomials which span the polynomial space of
 order \a P. Confusion may arise about the use of the term
 \a order. While the order, or \a degree, of the expansion
 basis corresponds to the maximal polynomial degree of the basis
 functions, the order of the method essentially refers to the accuracy
 of the approximation. More specifically, it depends on the convergence
 rate of the approximation with respect to mesh-refinement. It has been
 shown by Babuska and Suri (1994), that for a sufficiently smooth exact
 solution \f$u\in H^k(\Omega)\f$ (\f$k\geq P+1\f$), the error of the FEM
 approximation \f$u^{\delta}\f$ can be bounded by:
 
 \f[
 ||u-u^{\delta}||_{E}\leq Ch^P ||u||_k.
 \f]
 
 This implies that when decreasing the mesh-size \a h, the error of the
 approximation algebraically scales with the \f$P^{th}\f$ power of
 \a h. This can be formulated as:
 \f[
 ||u-u^{\delta}||_{E}=O(h^P).
 \f]
 If this holds, one generally classifies the method as a \f$P^{th}\f$-order
 FEM.  However, for non-smooth problems, i.e. \f$k<P+1\f$, the order of the
 approximation will in general be lower than \a P, the order of the
 expansion.
 
* \subsection HFEM h-version FEM
 
 A finite element method is said to be of \a h-type when the degree \a P
 of the piecewise polynomial basis functions is fixed and when any
 change of discretisation to enhance accuracy is done by means of a
 mesh refinement, that is, a reduction in \a h. Dependent on the
 problem, local refinement rather than global refinement may be
 desired. The \a h-version of the classical FEM
 employing linear basis functions can be classified as a first-order
 method when resolving smooth solutions.
 
* \subsection PFEM p-version FEM
 
 In contrast with the \a h-version FEM, finite element methods are said
 to be of \a p-type when the partitioning of domain is kept fixed and
 any change of discretisation is introduced through a modification in
 polynomial degree \a P. Again here, the polynomial degree may vary per
 element, particularly when the complexity of the problem requires
 local enrichment.  However, sometimes the term \a p-type FEM is merely
 used to indicated that a polynomial degree of \f$P>1\f$ is used.
 
* \subsubsection HPFEM hp-version FEM
 
 In the \a hp-version of the FEM, both the ideas of mesh refinement and
 degree enhancement are combined.
 
* \subsection SPEC The spectral method
 
 As opposed to the finite element methods which builds a solution from
 a sequence of local elemental approximations, spectral methods
 approximate the solution by a truncated series of \a global basis
 functions. Modern spectral methods, first presented by
 Gottlieb and Orzag (1977), involve the expansion of the solution into
 high-order orthogonal expansion, typically by employing Fourier,
 Chebyshev or Legendre series.
 
* \subsection SPECEL The spectral element method
 
 Patera in 1984 combined the high accuracy of the spectral
 methods with the geometric flexibility of the finite element method to
 form the spectral element method. The multi-elemental nature makes the
 spectral element method conceptually similar to the above mentioned
 high-order finite element. However, historically the term spectral
 element method has been used to refer to the high-order finite element
 method using a specific nodal expansion basis. The class of nodal
 higher-order finite elements which have become known as spectral
 elements, use the Lagrange polynomials through the zeros of the
 Gauss-Lobatto(-Legendre) polynomials.
 
* \subsection SPECHPEL The spectral/hp element method
 
 The spectral/hp element method, as its name suggests, incorporates
 both the multi-domain spectral methods as well as the more general
 high-order finite element methods. One can say that it encompasses all
 methods mentioned above. However, note that the term spectral/hp
 element method is mainly used in the field of fluid dynamics, while
 the terminology \a p  and \a hp -FEM originates from the area of
 structural mechanics. 
 
* \section GALERIK The Galerkin formulation
 
 Finite element methods typically use the Galerkin formulation to
 derive the weak form of the partial differential equation to be
 solved. We will primarily adopt the classical
 Galerkin formulation in combination with globally \f$C^0\f$ continuous
 spectral/hp element discretisations.
 
 To describe the Galerkin method, consider a steady \a linear
 differential equation in a domain \f$\Omega\f$ denoted by
 \f[
 L(u)=f,
 \f]
 subject to appropriate boundary conditions. In the Galerkin method,
 the weak form of this equation can be derived by pre-multiplying this
 equation with a test function \f$v\f$ and integrating the result over the
 entire domain \f$\Omega\f$ to arrive at: Find \f$u\in\mathcal{U}\f$ such that
 \f[
 \int_\Omega vL(u)d\boldsymbol{x}=\int_\Omega v f d\boldsymbol{x}, \quad \forall v\in\mathcal{V},
 \f]
 where \f$\mathcal{U}\f$ and \f$\mathcal{V}\f$ respectively are a suitably
 chosen trial and test space (in the traditional Galerkin method, one
 typically takes \f$\mathcal{U}=\mathcal{V}\f$). In case the inner product
 of \f$v\f$ and \f$\mathbb{L}(u)\f$ can be rewritten into a bi-linear form
 \f$a(v,u)\f$, this problem is often formulated more concisely as: Find \f$u\in\mathcal{U}\f$
 such that
 \f[
 a(v,u)=(v,f),\quad \forall v\in\mathcal{V},
 \f]
 where \f$(v,f)\f$ denotes the inner product of \f$v\f$ and \f$f\f$.  The next step
 in the classical Galerkin finite element method is the discretisation:
 rather than looking for the solution \f$u\f$ in the infinite dimensional
 function space \f$\mathcal{U}\f$, one is going to look for an approximate
 solution \f$u^\delta\f$ in the reduced finite dimensional function space
 \f$\mathcal{U}^\delta\subset\mathcal{U}\f$. Therefore we represent the approximate
 solution as a linear combination of basis functions \f$\Phi_n\f$ that span the
 space \f$\mathcal{U}^\delta\f$, i.e.
 \f[
 u^\delta=\sum_{n\in\mathcal{N}}\Phi_n\hat{u}_n.
 \f]
 Adopting a similar discretisation for the test functions \f$v\f$, the
 discrete problem to be solved is given as: Find \f$\hat{u}_n\f$
 \f$(n\in\mathcal{N})\f$ such that
 \f[
 \sum_{n\in\mathcal{N}}a(\Phi_m,\Phi_n)\hat{u}_n=(\Phi_m,f),\quad \forall m\in\mathcal{N}.
 \f]
 It is customary to describe this set of equations in matrix form as
 \f[
 \boldsymbol{A}\hat{\boldsymbol{u}}=\hat{\boldsymbol{f}}.
 \f]
 where \f$\hat{\boldsymbol{u}}\f$ is the vector of coefficients
 \f$\hat{u}_n\f$, \f$\boldsymbol{A}\f$ is the system matrix with elements
 \f[
 \boldsymbol{A}[m][n]=a(\Phi_m,\Phi_n)=\int_\Omega \Phi_mL(\Phi_n)d\boldsymbol{x},
 \f]
 and the vector \f$\hat{\boldsymbol{f}}\f$ is given by
 \f[
 \hat{\boldsymbol{f}}[m]=(\Phi_m,f)=\int_\Omega \Phi_mfd\boldsymbol{x}.
 \f]
 
 
 
*/