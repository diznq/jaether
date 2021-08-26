class Main {
    public static Main instance = new Main();

    public int attr1 = 123;
    public int attr2 = 234;

    public static Main getInstance(){
        return instance;
    }

    Main(){

    }

    public int computation(int n){
        int J = 0;
        int L = 10 + n;
        for(int I=0; I<L; I++){
            J += I * I;
        }
        return J;
    }

    public static void main(String[] args){
        long start = System.currentTimeMillis();
        Main instance = Main.getInstance();
        int s = 0;
        for(int i=0; i<100; i++){
            s += instance.computation(10);
        }
        long time = System.currentTimeMillis() - start;
        System.out.println(time);
        System.out.println(s);
    }
}