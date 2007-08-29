/************************************/
/* gestion d'un controleur flou     */
/*                                  */
/************************************/

#ifndef _FUZZY_H_
#define _FUZZY_H_

#ifndef Min
#define Min(_a,_b) ( (_a)<(_b) ? (_a) : (_b) )
#endif

#ifndef Max
#define Max(_a,_b) ( (_a)>(_b) ? (_a) : (_b) )
#endif

typedef float         	   	Reel32;
typedef Reel32 		  	   	Reel;
typedef double        	   	Reel64;
typedef unsigned char      	UInt8;
typedef UInt8 				Flags;
typedef unsigned short int 	UInt16;
typedef unsigned int       	UInt32;
typedef unsigned long      	UInt64;
typedef char          		Int8;
typedef short int     		Int16;
typedef int           		Int32;
typedef long          		Int64;

/**
 *
 */
typedef struct {
	Reel Center; // Parametre de la fonction d'appartenance.
  	Reel Width;
	unsigned short type;
  	//Reel ( * AppFonc )( Reel,Reel,Reel); // fonction d'appartenance.
  	/* FIXME */ //Reel GetDegApp(Reel Val); // renvoie le degre d'appartenance au sous ensemble flou
} SsEnsFlou;

/**
 *
 */
typedef struct {
  	unsigned char Drapo;
  	unsigned int NbSsEnsFlou; // Nombre de sous ensemble flou
  	SsEnsFlou **LSsEnsFlou; // pointeur des classes SsEnsFlou
  	/* FIXME */ //void GetDegreApp( Reel Valeur,Reel *DegApp);
} EnsFlou;

/**
 *
 */
typedef struct {
  	unsigned char Drapo; // variable permettant de savoir si les allocations ont fonctionnees
  	unsigned int **DivApp; // stocke les differents numero de sous ensembles flous
  	unsigned int NbEnsFlou; // nombre de sous ensembles flou.
  	unsigned int NbRegle; // nombre de regle
  	unsigned int *NbSsEnsFlou; // pour chaque ensemble flou nombre de sous ensembles flous
  //	void ( * GetDegVerite )(Reel* /*DegApp*/,Reel* /*DegVerite*/);
} OperEt;

/**
 *
 */
typedef struct {
  	Reel *DegApp; // vecteur contenant les degres d'appartenance au divers fonction d'appartenance.
  	Reel *DegVer; // vecteur contenant les degrees de verite au diverse regles flou
  	unsigned char Drapo;
  	unsigned int NbEnsFlou; // nombre d'ensemble flou et d'entree
  	EnsFlou **LEnsFlou; // vecteur de pointeur d'ensembleflou
  	unsigned int NbRegle; // nombre de regle du controleur.
  	OperEt *POperEt; // Pointeur sur la classe gerant le ET pour les regles.
  	Reel *Conclusions; // les valeurs des conclusions sont donnees par le chromosome ...
  	unsigned int *CumulSsEnsFlou; // cumul des differents sous ensembles flou
  	unsigned int NbSsEnsFlouTot; // Nombre Total de sous ensemble flous
  	/* FIXME */ //Reel Commande(Reel *LEntree);
} ControleFlou;

/**
 *
 */
SsEnsFlou *build_SsEnsFlou(Reel NCenter, Reel NWidth, unsigned short type/*Reel ( * NAppFonc )( Reel,Reel,Reel)*/);

/**
 *
 */
EnsFlou *build_EnsFlou(Reel UniMin, Reel UniMax, unsigned int NNbSsEnsFlou);

/**
 *
 */
ControleFlou *build_ControleFlou(unsigned int NbEntree, Reel *UniMin, Reel *UniMax, unsigned int *NNbSsEnsFlou);

/**
 *
 */
OperEt *build_OperEt(unsigned int NNbEnsFlou, unsigned int *NNbSsEnsFlou);

/**
 *
 */
void destroy_EnsFlou(EnsFlou *ptr);

/**
 *
 */
void /*EnsFlou::*/GetDegreApp(EnsFlou *ensflou, Reel Valeur, Reel *DegApp);  /* pre : DegApp est un tableau de Reel alloue */

/**
 *
 */
void /*OperEtLuka::*/GetDegVerite(OperEt *ptr, Reel *DegApp,Reel *DegVerite); /* pre : DegVerite est un tableau de Reel alloue */

/**
 *
 */
Reel /*ControleFlou::*/ Commande(ControleFlou *ptr, Reel *LEntree);

#endif
