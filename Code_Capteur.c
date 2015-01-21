

/*
 * 												Interface capteur
 *
 * Calcul et traitement de la vitesse.
 * Envoi vitesse sur port serie pour asservissement moteur
 * Envoi interruption au systeme affichage pour calibrage
 * Allume LED d'état.
 * 			-> P1.0 Vitesse < 10tr.sec-1
 * 			-> P1.1 Vitesse ~= 10tr.sec-1
 * 			-> P1.2 Vitesse > 10tr.sec-1
 * 			-> P1.3 Non detection de front montant capteur
 * 			
 * 			
 * 			 	   MSP430G2553
 * 	            -----------------          
 * LED orange->|P1.0          XIN|----\--\
 * LED verte-->|P1.1             |    QUARTZ (1MHZ // 125KHz)    
 * LED bleu--->|P1.2         XOUT|----/--/
 * LED rouge-->|P1.3             |        
 * Sortie Capt>|P1.4             |        
 *             |                 |       
 *             |              SDA|-------> Transmition data (I²C)
 *             |              SCL|-------> Transmition CLK (I²C)
 *             
 *             
 *                                              Ajout eventuel
 *
 * Si pas de traitement electronique à la sortie du capteur faire convertion analogique / numérique ( ADC10 ) 
 * Création d'une interruption sur valeur haute du ADC10
 * 
 * 												   A faire
 *
 * Création du code de transmition de data ==> Connaitre protocole utilisé
 *
 */

#include <msp430.h> 

#define LED_INF BIT0
#define LED_EQU BIT1
#define LED_SUP BIT2
#define LED_ERREUR BIT3

void transmit(int vit);
int etat=0; 									// Etat du compteur
int vitesse;

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    BCSCTL2 = SELS + DIVS_8; 					//SMCLK sycro sur Quartz 1MHz avec Diviseur de 8

    /*Déclaration entrée/sortie*/

	P1DIR |= LED_INF + LED_EQU + LED_SUP + LED_ERREUR;			//LED de contrôle en sortie
	P1DIR &= ~BIT4;								//Entrée capteur
	P1IE |= BIT4;								//Interruption sur entrée capteur
	P1IES &= ~BIT4; 							//Interruption sur front montant

	/*Initialisation TIMER*/
	
	TACTL = TASSEL_2 + MC_1 + TAIE;				// SMCLK, mode up jusqu'à TACCR0, interruption sur TACCR0 pas de diviseur
	TACCR0 = 37500;								// Correspond à 0.3sec avec Quartz de 1MHz


	return 0;
}


#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void){
	if(P1IFG & BIT4){							//Masquage sur le BIT4 pour éviter fausse mesure
		P1OUT &= ~LED_ERREUR;
		if(Etat==0){
			TACTL |= MC_1;						//On lance le timer
			Etat = 1;
		}else{
			Etat = 0;
			vitesse = TAR;						//Recuperation de la valeur du compteur au 2eme passage
			TACTL &= ~(BIT4 + BIT5);			//On Stop le timer
			transmit(vitesse);					//On envoi la valeur du compteur pour ne pas perdre d'info en compressant (nombre a virgule)
			if(vitesse =< 6100){				//Allumage de la LED appropriée
				P1OUT |= LED_INF;
				P1OUT &= ~(LED_SUP + LED_EQU);
			}else if(vitesse >=6350){
				P1OUT |= LED_SUP;
				P1OUT &= ~(LED_INF + LED_EQU);
			}else{
				P1OUT |= LED_EQU;
				P1OUT &= ~(LED_INF + LED_SUP);
			}
		}
	}
}

#pragma vector=TIMERA0_VECTOR 
__interrupt void Timer_A (void){
	if(TACTL & TAIFG){							//On vérifie si c'est bien l'interruption TAIFG
		P1OUT |= LED_ERREUR;							//Allumage de la LED d'erreur
	}
}


void transmit(int vit){
	//transmition par port série de la vitesse
}
