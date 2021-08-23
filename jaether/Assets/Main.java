class Main {

    public int count = 0;
    public int alt = 0;
    public int start = 0;

    Main(){
        this.count = calc(1000);
        this.alt = this.count / 10;
    }

    public int sq(int i){
        return i*i;
    }

    public int calc(int p){
        int i = 0;
        for(int j = 0; j < p; j++){
            i += j;
            System.out.println(sq(j));
        }
        return sq(i);
    }

    public static void main(String[] args){
        Main obj = new Main();
        obj.start = 123;
        System.out.println(obj.count + obj.start);
        System.out.println(obj.alt);
    }
}