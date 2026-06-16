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
private:
    vector<Block> cadena;
    int dificultad;
    Blockchain() {
        dificultad = 1;
        crearBloqueGenesis();
    }


    void crearBloqueGenesis() {
        vector<Voto> vacio;
        Block genesis(0, "0", vacio);
        cadena.push_back(genesis);
    }
public:

    Blockchain(const Blockchain&) = default;
    Blockchain& operator=(const Blockchain&) = default;
    static Blockchain& getInstance() {
        static Blockchain instancia;
        return instancia;
    }
    static bool sonIguales(const Blockchain& a, const Blockchain& b) {
        if (a.cadena.size() != b.cadena.size()) return false;
        for (size_t i = 0; i < a.cadena.size(); i++) {
            Block ba = a.cadena[i], bb = b.cadena[i];
            if (ba.calcularhash() != bb.calcularhash()) return false;
        }
        return true;
    }

    void mineBlock(Block& bloque, int dificultad) {
        string objetivo(dificultad, '0');

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

            if (actual.current_hash != actual.calcularhash()) {
                return false;
            }

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
    void _forzarCambioVoto(int idxBloque, int idxVoto, string nuevaOpcion) {
        cadena[idxBloque].votos[idxVoto].opcionelegida = nuevaOpcion;
    }

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
    Blockchain bc;
public:
    MesaElectoral(int ind) : indice(ind), bc(Blockchain::getInstance()) {}
    Blockchain& getCadena() { return bc; }
    int getIndice() const { return indice; }
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

int main() {
    MesaElectoral mesa1(1), mesa2(2), mesa3(3);
    CentroElectoralSubject centro;
    centro.attach(mesa1); centro.attach(mesa2); centro.attach(mesa3);

    // --- bloque 1 minado por mesa1 ---
    vector<Voto> votos1 = { Voto("voter_001","Candidato A"),
                            Voto("voter_002","Candidato B") };
    Blockchain& cad1 = mesa1.getCadena();
    Block b1(cad1.getSize(), cad1.getUltimoBloque().current_hash, votos1);
    cad1.mineBlock(b1, cad1.getDificultad());
    centro.notificarNuevoBloque(b1);

    // --- bloque 2 minado por mesa2 ---
    vector<Voto> votos2 = { Voto("voter_003","Candidato A"),
                            Voto("voter_004","Candidato A") };
    Blockchain& cad2 = mesa2.getCadena();
    Block b2(cad2.getSize(), cad2.getUltimoBloque().current_hash, votos2);
    cad2.mineBlock(b2, cad2.getDificultad());
    centro.notificarNuevoBloque(b2);

    // --- bloque 3 minado por mesa3 ---
    vector<Voto> votos3 = { Voto("voter_005","Candidato B"),
                            Voto("voter_006","Candidato C") };
    Blockchain& cad3 = mesa3.getCadena();
    Block b3(cad3.getSize(), cad3.getUltimoBloque().current_hash, votos3);
    cad3.mineBlock(b3, cad3.getDificultad());
    centro.notificarNuevoBloque(b3);

    // las 3 cadenas
    mesa1.getCadena().imprimirCadena();
    mesa2.getCadena().imprimirCadena();
    mesa3.getCadena().imprimirCadena();

    // Estado normal
    cout << "\n--- Estado normal ---\n";
    cout << "mesa1 valida? "   << (mesa1.getCadena().isChainValid() ? "SI":"NO") << endl;
    cout << "mesa2 valida? "   << (mesa2.getCadena().isChainValid() ? "SI":"NO") << endl;
    cout << "mesa3 valida? "   << (mesa3.getCadena().isChainValid() ? "SI":"NO") << endl;
    cout << "mesa1 == mesa2? " << (Blockchain::sonIguales(mesa1.getCadena(), mesa2.getCadena()) ? "SI":"NO") << endl;
    cout << "mesa1 == mesa3? " << (Blockchain::sonIguales(mesa1.getCadena(), mesa3.getCadena()) ? "SI":"NO") << endl;

    // --- alguien manipula la copia de mesa1 ---
    mesa1.getCadena()._forzarCambioVoto(1, 0, "Candidato C");

    cout << "\n--- Tras manipular mesa1 ---\n";
    cout << "mesa1 valida? "   << (mesa1.getCadena().isChainValid() ? "SI":"NO") << endl;
    cout << "mesa2 valida? "   << (mesa2.getCadena().isChainValid() ? "SI":"NO") << endl;
    cout << "mesa3 valida? "   << (mesa3.getCadena().isChainValid() ? "SI":"NO") << endl;
    cout << "mesa1 == mesa2? " << (Blockchain::sonIguales(mesa1.getCadena(), mesa2.getCadena()) ? "SI":"NO") << endl;
    cout << "mesa1 == mesa3? " << (Blockchain::sonIguales(mesa1.getCadena(), mesa3.getCadena()) ? "SI":"NO") << endl;

    return 0;}