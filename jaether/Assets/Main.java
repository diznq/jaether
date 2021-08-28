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

    public int fuc(int n){
        if(n<=1) return 1;
        return n + fuc(n - 1);
    }

    public static void main(String[] args){
        long start = System.currentTimeMillis();
        Main instance = Main.getInstance();
        int s = 0;
        int k = 0;
        int[] results = new int[10000];
        for(int j=0; j<100; j++){
            for(int i=0; i<100; i++, k++){
                results[k] = instance.fuc(10 + i);
            }
        }
        for(int i=0; i<results.length; i++){
            s += results[i];
        }
        long time = System.currentTimeMillis() - start;
        //for(int i=0; i<results.length; i++){
        //    System.out.println(results[i]);
        //}
        System.out.println(time);
        System.out.println(s);
    }
}