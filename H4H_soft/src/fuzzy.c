/*******************************************
Definition des fonctions necessaire au SsEnsFlou :

Reel Triangle(Center,Largeur,Valeur);
Reel TrapezeDecr(Center,Largeur,Valeur);
Reel TrapezeCroi(Center,Largeur,Valeur);
*******************************************/

//#include "stdafx.h"
#include <stdlib.h>
#include "fuzzy.h"

/**
 *
 */
Reel Triangle(Reel Center, Reel Largeur, Reel Valeur) {       /* pre : largeur >0 */
  	Valeur -= Center;
  	if (Valeur < 0)
  		Valeur =- Valeur; // fonction paire
  	// normalisation :
  	Valeur /= Largeur; 
  	Reel Resultat;
  	Resultat = Max(0, (1 - Valeur));
  	return Resultat;
}

/**
 *
 */
Reel TrapezeDecr(Reel Center, Reel Largeur, Reel Valeur) {
  	Valeur -= Center;
  	if (Valeur < 0)
    	return 1;
  	Valeur /= Largeur;
  	Reel Resultat;
  	Resultat = Max(0, (1 - Valeur));
  	return Resultat;
}

/**
 *
 */
Reel TrapezeCroi(Reel Center, Reel Largeur, Reel Valeur) {
  	Valeur -= Center;
  	if (Valeur > 0)
    	return 1;
  	Valeur /= -Largeur;
  	Reel Resultat;
  	Resultat = Max(0, (1 - Valeur));
  	return Resultat;
}

/**
 *
 */
SsEnsFlou *build_SsEnsFlou(Reel NCenter, Reel NWidth, unsigned short type/*Reel ( * NAppFonc )( Reel,Reel,Reel)*/) {
	SsEnsFlou *res = (SsEnsFlou*)malloc(sizeof (SsEnsFlou));
  	res->Center = NCenter;
  	res->Width = NWidth;
	res->type = type;
  	//res->AppFonc = NAppFonc;
	return res;
}

/**
 *
 */
void /*EnsFlou::*/GetDegreApp(EnsFlou *ensflou, Reel Valeur, Reel *DegApp) {  /* pre : DegApp est un tableau de Reel alloue */
  	// range les valeurs des  degres d'appartenance dans DegApp
  	Reel *PDegApp=DegApp;
  	SsEnsFlou **PLSsEnsFlou = ensflou->LSsEnsFlou;
	unsigned int GDi;
  	for(GDi = 0; GDi < ensflou->NbSsEnsFlou; ++GDi, ++PDegApp, ++PLSsEnsFlou) {
		switch ((*PLSsEnsFlou)->type) {
			case 0 : *PDegApp = Triangle((*PLSsEnsFlou)->Center,(*PLSsEnsFlou)->Width,Valeur); break;
			case 1 : *PDegApp = TrapezeDecr((*PLSsEnsFlou)->Center,(*PLSsEnsFlou)->Width,Valeur); break;
			case 2 : *PDegApp = TrapezeCroi((*PLSsEnsFlou)->Center,(*PLSsEnsFlou)->Width,Valeur); break;
		};
    	//*PDegApp = (*PLSsEnsFlou)->AppFonc((*PLSsEnsFlou)->Center,(*PLSsEnsFlou)->Width,Valeur);
	}
}

/**
 *
 */
EnsFlou *build_EnsFlou(Reel UniMin, Reel UniMax, unsigned int NNbSsEnsFlou) {     /* pre : NNbSsEnsFlou>=2    (Umax-Umin)>0  */
	EnsFlou *res = (EnsFlou*)malloc(sizeof (EnsFlou));
  	res->NbSsEnsFlou = NNbSsEnsFlou;
  	res->Drapo = 1;
  	res->LSsEnsFlou = (SsEnsFlou**)malloc (sizeof (SsEnsFlou*) * res->NbSsEnsFlou);
  	if (res->LSsEnsFlou == NULL)
    	return NULL;
  	unsigned int Inip;
  	Reel LargSsEF = (UniMax - UniMin) / (res->NbSsEnsFlou - 1);
  	Reel CumulCenter = UniMin;

  	// premiere fonction d'appartenance : Trapeze decroissant :
  	res->LSsEnsFlou[0] = build_SsEnsFlou(CumulCenter, LargSsEF, 1);
  	if (res->LSsEnsFlou[0] == NULL)
    	return NULL;
  	CumulCenter += LargSsEF;
  
  // entre la premier et la derniere fonction d'appartenance : Triangle
  	for (Inip = 1; Inip < (res->NbSsEnsFlou - 1); Inip++, CumulCenter += LargSsEF) {
      	res->LSsEnsFlou[Inip] = build_SsEnsFlou(CumulCenter, LargSsEF, 0);
      	if (res->LSsEnsFlou[Inip] == NULL)
		return NULL;
    }
  
  	// derniere fonction d'appartenance : trapeze croissant
  	res->LSsEnsFlou[res->NbSsEnsFlou - 1] = build_SsEnsFlou(UniMax, LargSsEF, 2);
  	if (res->LSsEnsFlou[res->NbSsEnsFlou - 1] == NULL)
    	return NULL;
  	res->Drapo = 0;
	return res;
}

/**
 *
 */
void destroy_EnsFlou(EnsFlou *ptr) {
	unsigned int i;
	for (i = 0; i < ptr->NbSsEnsFlou; ++i)
		free(ptr->LSsEnsFlou[i]);
	free(ptr);
}

/*************************************

Definition des methodes de OperEt

*************************************/

/**
 *
 */
OperEt *build_OperEt(unsigned int NNbEnsFlou, unsigned int *NNbSsEnsFlou) {
	OperEt *res = (OperEt*)malloc(sizeof (OperEt));
	// NNbSsEnsFlou contient le nombre de sous ensembles flou pour chaque ensemble flou
	res->NbEnsFlou = NNbEnsFlou;
	res->Drapo = 1;

	// initialisation des pointeurs :
	res->NbSsEnsFlou = (unsigned int*)malloc(sizeof (unsigned int) * res->NbEnsFlou);
	if (res->NbSsEnsFlou == NULL) {
		// l'allocation n'a pas fonctionnee : Drapo est toujours a 1
		return NULL;
	}
	// recopie de NNbSsEnsFlou dans NbSsEnsFlou et calcul du nombre de regle
	res->NbRegle = 1;
	unsigned int Recopie;
	for (Recopie = 0; Recopie < res->NbEnsFlou; ++Recopie) {
		res->NbSsEnsFlou[Recopie] = NNbSsEnsFlou[Recopie];
		res->NbRegle *= res->NbSsEnsFlou[Recopie];
	}

	// allocation de DivApp
	res->DivApp = (unsigned int**)malloc(sizeof(unsigned int*) * res->NbRegle);
	if (res->DivApp == NULL) {
		// l'allocation n'a pas fonctionnee : Drapo est toujours a 1
		return NULL;
	}
	// initialisation a NULL pour le destructeur si l'allocation ne fonctionne pas
	unsigned int Allie; 

	// allocation :
	for (Allie = 0; Allie < res->NbRegle; ++Allie) {
    	res->DivApp[Allie] = (unsigned int*)malloc(sizeof (unsigned int) * res->NbEnsFlou);
    	if (res->DivApp[Allie] == NULL)
			return NULL;
  	}
	// initialisation des differentes valeurs de DivApp.
	// allocation (pour le calcul) d'un vecteur contenant le numero des premiers
	// degres d'appartenance des differents ensembles flous :

	int *TempPrem = (int*)malloc(sizeof (int) * res->NbEnsFlou);
	if (TempPrem == NULL)
		return NULL;
	int *TempComp = (int*)malloc(sizeof (int) * res->NbEnsFlou);
	if (TempComp == NULL)
  		return NULL;
	unsigned CumulSsEF = 0; // cumul des differents sous ensembles flous
	unsigned int TempEF;
	for (TempEF = 0; TempEF < res->NbEnsFlou; TempEF++) {
    	TempPrem[TempEF] = CumulSsEF;
    	TempComp[TempEF] = CumulSsEF; 
    	CumulSsEF += res->NbSsEnsFlou[TempEF];
  	}
	unsigned int ToutEi; // compteur pour tout les ensembles flous
	unsigned int Reci; // compteur pour la recopie de TempComp dans DivApp
	unsigned int Regli;
	/*
	principe de l'algo : 
	on passe en revue toutes les combinaisons de regles possibles 
	et on les stockes dans DivApp
	*/

	for (Regli = 0; Regli < res->NbRegle; Regli++, (TempComp[0])++) {
    	for (ToutEi = 0; ToutEi < res->NbEnsFlou - 1; ToutEi++) {
			if (TempComp[ToutEi] >= TempPrem[ToutEi + 1]) {
	    		TempComp[ToutEi] = TempPrem[ToutEi];
	    		(TempComp[ToutEi + 1])++;
	  		}
			else
	  			ToutEi = res->NbEnsFlou;
     	}
    	// recopie de TempComp dans DivApp
    	for (Reci = 0; Reci < res->NbEnsFlou; Reci++)
			res->DivApp[Regli][Reci] = TempComp[Reci];
	}

	free(TempComp);
	free(TempPrem);
	res->Drapo = 0;
	return res;
}

/**
 *
 */
ControleFlou *build_ControleFlou(unsigned int NbEntree, Reel *UniMin, Reel *UniMax, unsigned int *NNbSsEnsFlou) {
	ControleFlou *res = (ControleFlou*)malloc(sizeof (ControleFlou));
  	// initialisations preliminaires de Drapo, de NbEnsFlou et des differents pointeurs
  	res->Drapo = 1;
  	res->NbEnsFlou = NbEntree;
  	res->LEnsFlou = (EnsFlou**)malloc(sizeof (EnsFlou*) * res->NbEnsFlou);
  	if (res->LEnsFlou == NULL)
    	{printf("Bad Trip in build_ControleFlou\n"); return NULL;}
  // Initialisation des differents ensembles flou : LEnsFlou
  	unsigned int Ensi;
  	for(Ensi = 0; Ensi < res->NbEnsFlou; Ensi++) {
      	res->LEnsFlou[Ensi] = build_EnsFlou(UniMin[Ensi],UniMax[Ensi],NNbSsEnsFlou[Ensi]);
      	if (res->LEnsFlou[Ensi] == NULL)
			{printf("Bad Trip in build_ControleFlou\n"); return NULL;}
      	if (res->LEnsFlou[Ensi]->Drapo)
		{printf("Bad Trip in build_ControleFlou\n"); return NULL;}
    }
  
  // initialisation de l'OperateurET, de NbRegle et de DegVer
  
  	res->POperEt = build_OperEt(res->NbEnsFlou, NNbSsEnsFlou);
  	if (res->POperEt == NULL)
    	{printf("Bad Trip in build_ControleFlou\n"); return NULL;}
  	if (res->POperEt->Drapo)
    	{printf("Bad Trip in build_ControleFlou\n"); return NULL;}
  
  	res->NbRegle = res->POperEt->NbRegle;
  
  	res->DegVer = (Reel*)malloc(sizeof(Reel) * res->NbRegle);
  	if (res->DegVer == NULL)
    	{printf("Bad Trip in build_ControleFlou\n"); return NULL;}
  
  // Initialisation de CumulSsEnsFlou, de NbSsEnsFlouTot et de DegApp
  
  	res->CumulSsEnsFlou = (unsigned int*)malloc(sizeof(unsigned int) * res->NbEnsFlou);
  	unsigned CumulSsEF = 0; // cumul des differents sous ensembles flous
	unsigned int TempEF;
  	for (TempEF = 0; TempEF < res->NbEnsFlou; TempEF++) {
      	res->CumulSsEnsFlou[TempEF] = CumulSsEF;
      	CumulSsEF += NNbSsEnsFlou[TempEF];
    }
  
  
  	res->NbSsEnsFlouTot=CumulSsEF;
  	res->DegApp = (Reel*)malloc(sizeof(Reel) * CumulSsEF);
  	if(res->DegApp == NULL)
    	{printf("Bad Trip in build_ControleFlou\n"); return NULL;}

  	res->Drapo = 0; // tout a fonctionne correctement.
	return res; 
}


/**
 *
 */
void /*OperEtLuka::*/GetDegVerite(OperEt *ptr, Reel *DegApp,Reel *DegVerite) {
	/*
    DegApp correspond au differents
    degres d'appartenance au differents sous ensembles flou.
    les degres d'appartenance sont mis les uns apres les autres dans le vecteur
    */
  	Reel SommeDeg;
  	unsigned int **PDivApp = ptr->DivApp; // optimisation
  	unsigned int *TempDivApp; // optimisation
  	Reel *PDegVerite = DegVerite; // optimisation
	unsigned int Riz;
	for (Riz = 0; Riz < ptr->NbRegle; Riz++, PDivApp++,PDegVerite++) {
      	SommeDeg = 1;
		TempDivApp = *PDivApp;
		unsigned int Ssi;
		for (Ssi = 0; Ssi < ptr->NbEnsFlou; Ssi++, TempDivApp++) {
			// on  a  DivApp[Riz][Ssi]=*TempDivApp
	    	SommeDeg = Min(SommeDeg, DegApp[*TempDivApp]); // Cas du Min
	    	// ou  SommeDeg*=DegApp[ *TempDivApp ] 
	  	}
		// DegVerite[Riz]=SommeDeg; or PDegVerite=DegVerite[Riz];
		*PDegVerite = SommeDeg;
	}
}

/**
 *
 */
Reel /*ControleFlou::*/ Commande(ControleFlou *ptr, Reel *LEntree) {
  	// Premiere partie : obtention des divers degres d'appartenance :
  	Reel *PLEntree = LEntree; // optimisation
  	unsigned int *PCumulSsEnsFlou = ptr->CumulSsEnsFlou; // optimisation
  	EnsFlou **PLEnsFlou = ptr->LEnsFlou;  // optimisation
	unsigned int Degi;
  	for(Degi = 0; Degi < ptr->NbEnsFlou; Degi++, PLEnsFlou++, PLEntree++, PCumulSsEnsFlou++)
    	GetDegreApp(*PLEnsFlou, *PLEntree, ptr->DegApp + *PCumulSsEnsFlou);
  	// deuxieme partie : obtention des divers degres de verites des regles :
	GetDegVerite(ptr->POperEt, ptr->DegApp, ptr->DegVer);
  	// troisieme partie : calcul de la conclusion : 
  	// le resultat est la moyenne des conclusions ponderee par leurs degres de verite
  	// a : calcul du cumul des degree de verite et du cumul des produits des conclusions par leur degre de verite associe :
  	Reel CumulDegVer = 0;
  	Reel CumulDegVerfCon = 0;
  	Reel *PDegVer = ptr->DegVer; // optimisation
  	Reel *PConclusions = ptr->Conclusions; // optimisation
 	unsigned int Devi;
  	for (Devi = 0; Devi < ptr->NbRegle; Devi++, PDegVer++, PConclusions++) {
      	CumulDegVer += *PDegVer;
      	CumulDegVerfCon += (*PDegVer) * (*PConclusions);
    }
  	// CumulDegVer n'est pas nul
  	return (CumulDegVerfCon / CumulDegVer);
}

