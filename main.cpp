#include <iostream>
#include <string>
#include <vector>
using namespace std;
struct Voto {
    string votanteid; //ej votante_001
    string opcionelegida; //ej opcion A o opcion B

    Voto(string vid,string opeleg) {
        this->votanteid = vid;
        this->opcionelegida = opeleg;

    }
    string getvotanteid() {
        return this->votanteid;
    }
    string getopcionelegida() {
        return this->opcionelegida;
    }
};

//funcion que convierte cualquier tipo de string a otro hasheado
string hashSimple(const string & datos) {
    unsigned long hash = 5381; //usigned hace que la variable solo pueda usar valores
    //positivos o cero
    for (char c : datos) { //Por cada letra actualiza el valor de hash
        hash = ((hash << 5) + hash) + c; //<< multiplica por 32 (<<5=2**5) (forma que usan las computadoras)
    } //+hash = has*32 osea en realidad se hace en hash*33 luego se le suma c
    //En resumen: Por cada letra, el hash actual se multiplica por 33 y se le suma el valor de la letra.
    //se convierte a string en base hexadecimal
    string resultado = "";
    unsigned long temp=hash; //variable temporal que tiene el hash aterior
    const string hex= "0123456789abcdef"; //base 16
    do {
        resultado= hex[temp%16]+resultado;
        temp=temp/16;//armamos la cadena, concatenamos a la izquierda
        // calculamos hex[indice] y a eso le sumamos el anterior
        //luego divido entre 16 para pasar a la otro posición
    } while (temp>0); //cuando el numero se divide tanto habra un punto el cual llegue a 0
    //alli para el bucle
    return resultado;
}
struct Block {
    int indice; // indice del bloque
    string previous_hash; //el hash previo al bloque
    //considerar que para el primero es 00000
    vector <Voto> votos; //Lista de votos
    int nonce; //empieza en 0
    string current_hash; //el hash de este bloque
    Block(int ind, string ph, vector <Voto> lista_vt) {
        this->indice = ind;
        this->previous_hash = ph;
        this->votos = lista_vt;
        this->nonce = 0; //empieza en 0
        this->current_hash = calcularhash(); //se calcula
    }

    string calcularhash() {
        string datos = to_string(indice) + previous_hash +to_string(nonce); //indice-hashprevio-nonce
        for (Voto v : votos) { //ahora para cada voto hasheamos segun corresponda
            datos+= v.getvotanteid()+v.getopcionelegida();
        }
        return hashSimple(datos);
    }
};



int main(){
    //Probando algunos votos
    vector<Voto> votos;
    votos.push_back(Voto("voter_001", "Candidato A"));
    votos.push_back(Voto("voter_002", "Candidato B"));
    votos.push_back(Voto("voter_003", "Candidato A"));

    // Bloque
    Block b(1, "0000000000", votos);

    // Verificacion que se calcula hash
    cout << "Indice:    " << b.indice << endl;
    cout << "PrevHash:  " << b.previous_hash << endl;
    cout << "Nonce:     " << b.nonce << endl;
    cout << "Hash:      " << b.current_hash << endl;

    return 0;

};

