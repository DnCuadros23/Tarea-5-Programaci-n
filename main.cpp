#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
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
    while (resultado.length() < 8) {
        resultado = "0" + resultado;
    }
    return resultado;
}
struct Block {
    int indice;
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
class Blockchain {
    vector<Block> cadena;
    int dificultad;
    Blockchain() {
        dificultad = 1; // o el que quieras por defecto
        crearBloqueGenesis();
    }

    // Bloquear copia y asignación
    Blockchain(const Blockchain&) = delete;
    Blockchain& operator=(const Blockchain&) = delete;

    void crearBloqueGenesis() {
        vector<Voto> vacio;
        Block genesis(0, "0", vacio);
        cadena.push_back(genesis);
    }
public:
    static Blockchain& getInstance() {
        static Blockchain instancia;
        return instancia;
    }

    void mineBlock(Block& bloque, int dificultad) {
        string objetivo(dificultad, '0');  // ej. dificultad=3 → "000"

        while (bloque.current_hash.substr(0, dificultad) != objetivo) {
            bloque.nonce++;
            bloque.current_hash = bloque.calcularhash();
        }

        cout << "Bloque minado: " << bloque.current_hash << endl;
    }
    bool isChainValid() {
        for (int i = 1; i < (int)cadena.size(); i++) {
            Block& actual = cadena[i];
            Block& anterior = cadena[i-1];

            // (a) ¿El hash guardado coincide con el recalculado?
            //     Si alguien cambió un voto, el hash ya no cuadra.
            if (actual.current_hash != actual.calcularhash()) {
                return false;
            }

            // (b) ¿El previous_hash apunta correctamente al anterior?
            if (actual.previous_hash != anterior.current_hash) {
                return false;
            }
        }
        return true;
    }
    // Añade un bloque ya minado a la cadena
    void agregarBloque(Block nuevo) {
        cadena.push_back(nuevo);
    }

    // Helpers para que los otros integrantes los usen
    Block getUltimoBloque() {
        return cadena.back();
    }

    int getSize() {
        return cadena.size();
    }

    int getDificultad() {
        return dificultad;
    }

    void imprimirCadena() {
        cout << "\n========= CADENA DE BLOQUES =========\n";
        for (Block& b : cadena) {
            cout << "---- Bloque " << b.indice << " ----\n";
            cout << "PrevHash: " << b.previous_hash << "\n";
            cout << "Nonce:    " << b.nonce << "\n";
            cout << "Hash:     " << b.current_hash << "\n";
            cout << "Votos en este bloque:\n";
            for (Voto& v : b.votos) {
                cout << "   - " << v.getvotanteid()
                     << " -> " << v.getopcionelegida() << "\n";
            }
            cout << "\n";
        }
        cout << "=====================================\n";
    }};

class MesaElectoralObserver {
    public:
    virtual void update(Block& bloque) = 0;};

class MesaElectoral : public MesaElectoralObserver {
private:
    int indice;
    Blockchain& bc;
public:
    MesaElectoral(int ind) : bc(Blockchain::getInstance()), indice(ind) {}
    void update(Block& bloque) override {
        cout << "La mesa " << indice << " recibio el bloque " << bloque.indice << endl;
        if (bc.getUltimoBloque().current_hash == bloque.current_hash) {
            cout << "El bloque " << bloque.indice << " ya estaba en la cadena" << endl;
            return;}

        if (bloque.current_hash == bloque.calcularhash()) {
            bc.agregarBloque(bloque);
            cout << "Se agrego el bloque " << bloque.indice << endl;}

        else cout << "No se pudo agregar el bloque " << bloque.indice << endl;}};

    class CentroElectoralSubject {
    private:
        vector<MesaElectoralObserver*> observers;
    public:
        CentroElectoralSubject() = default;

        void attach(MesaElectoralObserver &mesa) {
            observers.push_back(&mesa);}

        void notificarNuevoBloque(Block &bloque) {
            for_each(observers.begin(), observers.end(), [&bloque](MesaElectoralObserver* observer) {
                observer->update(bloque);});}};


    int main(){
        Blockchain& bc = Blockchain::getInstance();

        //Creamos las mesas
        MesaElectoral mesa1(1);
        MesaElectoral mesa2(2);
        MesaElectoral mesa3(3);

        // Creamos centro electoral y registramos las mesas
        CentroElectoralSubject centro;
        centro.attach(mesa1);
        centro.attach(mesa2);
        centro.attach(mesa3);

        // mesa 1 registra votos y mina un bloque
        vector<Voto> votos1;
        votos1.push_back(Voto("voter_001", "Candidato A"));
        votos1.push_back(Voto("voter_002", "Candidato B"));

        Block b1(bc.getSize(), bc.getUltimoBloque().current_hash, votos1);
        bc.mineBlock(b1, bc.getDificultad());

        cout << "\n--- Mesa 1 mino el bloque " << b1.indice << ", notificando a la red ---\n" << endl;

        // Notificamos el bloque nuevo a todas las mesas que registramos
        centro.notificarNuevoBloque(b1); // todas las mesasverifican el bloque y lo agregan a su copia de la cadena

        //resultado
        bc.imprimirCadena();
        cout << "Cadena valida? " << (bc.isChainValid() ? "SI" : "NO") << endl;

        return 0;}