package example;

import java.math.BigInteger;
import java.util.Random;

class Main {

    public static void main(String[] args){
        Random random = new Random();
        System.out.println(BigInteger.probablePrime(256, random).toString(10));
    }
}