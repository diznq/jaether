class Main {

    public static int sq(int i){
        return i*i;
    }

    public static int calc(int p){
        int i = 0;
        for(int j = 0; j < p; j++){
            i += j;
            System.out.println(sq(j));
        }
        return sq(i);
    }

    public static void main(String[] args){
        System.out.println(calc(100));
    }
}