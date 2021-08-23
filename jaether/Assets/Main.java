class Main {

    public int count = 0;
    public int alt = 0;
    public int start = 0;

    public static int ops = 10;

    public static Main instance = new Main();

    public static Main getInstance(){
        return instance;
    }

    Main(){
        this.count = calc(10);
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
            Main.ops += 1;
        }
        return sq(i);
    }

    public static void main(String[] args){
        Main obj = Main.getInstance();
        obj.start = 123;
        int result = obj.calc(10);
        System.out.println(obj.count + obj.start);
        System.out.println(obj.alt);
        System.out.println(result);
        System.out.println(Main.ops);
    }
}