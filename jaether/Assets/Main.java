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
        int j = 0;
        int l = 1000 + n;
        for(int i=0; i<l; i++){
            j += i * i;
        }
        return j;
    }

    public static void main(String[] args){
        long start = System.currentTimeMillis();
        Main instance = Main.getInstance();
        int s = 0;
        for(int k=0; k<100; k++){
            for(int i=0; i<100; i++){
                s += instance.computation(i);
            }
        }
        long time = System.currentTimeMillis() - start;
        instance.attr1 += (int)time;
        System.out.println(instance.attr1);
        System.out.println(s);
    }
}