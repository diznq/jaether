class Main {

    public static int sq(int i){
        return i*i;
    }

    public static int calc(){
        int i = 0;
        for(int j = 0; j < 50; j++){
            i += j;
            System.out.println(i);
        }
        return sq(i);
    }

    public static void main(String[] args){
        int i = 0;
        for(int j = 0; j < 50; j++){
            i += j;
        }
        System.out.println(i);
    }
}