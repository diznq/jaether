package example;

public class Rectangle extends Square {
    public int c = 30;
    public static int C = 30;
    Rectangle(){
        super();
    }

    @Override
    public int area(){
        return 4;
    }
}