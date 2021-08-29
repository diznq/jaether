package example;

class Main {
    public static Main instance = new Main();

    public int attr1 = 123;
    public int attr2 = 234;

    public static Main getInstance(){
        return instance;
    }

    Main(){
        
    }

    public int fuc(int n){
        if(n<=1) return 1;
        return n + fuc(n - 1);
    }

    public String getString(int n){
        return String.valueOf(n);
    }

    public static void main(String[] args){
        Main main = Main.getInstance();
        String names[] = new String[]{"Čau",  "핐핑"};
        for(String name : names){
            System.out.println(name);
        }
    }
}