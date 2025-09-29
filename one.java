import java.util.Scanner;

public class one {

    public static String toBinary(int value) {
        return String.format("%8s", Integer.toBinaryString(value)).replace(' ', '0');
    }

    public static String intToIP(int ip) {
        return ((ip >>> 24) & 0xff) + "." +
                ((ip >>> 16) & 0xff) + "." +
                ((ip >>> 8) & 0xff) + "." +
                (ip & 0xff);
    }

    public static String intToBinaryIP(int ip) {
        return toBinary((ip >>> 24) & 0xff) + " " +
                toBinary((ip >>> 16) & 0xff) + " " +
                toBinary((ip >>> 8) & 0xff) + " " +
                toBinary(ip & 0xff);
    }

    public static String getSubnetMask(int prefix) {
        int mask = 0xffffffff << (32 - prefix);
        return intToIP(mask);
    }

    public static String getSubnetMaskBinary(int prefix) {
        int mask = 0xffffffff << (32 - prefix);
        return intToBinaryIP(mask);
    }

    public static void calculateSubnetDetails(String ip, int prefix, int requiredSubnets) {
        String[] parts = ip.split("\\.");
        int ipNum = (Integer.parseInt(parts[0]) << 24) |
                (Integer.parseInt(parts[1]) << 16) |
                (Integer.parseInt(parts[2]) << 8) |
                Integer.parseInt(parts[3]);

        System.out.println("\n--- Original Network Details ---");
        System.out.println("IP Address (Decimal): " + ip);
        System.out.println("IP Address (Binary): " + intToBinaryIP(ipNum));
        System.out.println("Subnet Mask (Decimal): " + getSubnetMask(prefix));
        System.out.println("Subnet Mask (Binary): " + getSubnetMaskBinary(prefix));
        System.out.println("Prefix Length: /" + prefix);

        int hostBits = 32 - prefix;
        int totalHosts = (int) Math.pow(2, hostBits) - 2;
        System.out.println("Total Hosts (without subnetting): " + totalHosts);

        int bitsNeeded = (int) Math.ceil(Math.log(requiredSubnets) / Math.log(2));
        int newPrefix = prefix + bitsNeeded;
        int newHostBits = 32 - newPrefix;
        int hostsPerSubnet = (int) Math.pow(2, newHostBits) - 2;
        int totalSubnets = (int) Math.pow(2, bitsNeeded);

        System.out.println("\n--- Subnetting Details ---");
        System.out.println("Number of subnets required: " + requiredSubnets);
        System.out.println("Binary (subnet count): " + Integer.toBinaryString(requiredSubnets));
        System.out.println("Bits borrowed: " + bitsNeeded);
        System.out.println("New Prefix Length: /" + newPrefix);
        System.out.println("New Subnet Mask (Decimal): " + getSubnetMask(newPrefix));
        System.out.println("New Subnet Mask (Binary): " + getSubnetMaskBinary(newPrefix));
        System.out.println("Total Subnets Possible: " + totalSubnets);
        System.out.println("Hosts per Subnet: " + hostsPerSubnet);
        int blockSize = (int) Math.pow(2, newHostBits);
        System.out.println("\n--- Subnet Ranges ---");
        for (int i = 0; i < requiredSubnets; i++) {
            int networkAddress = (ipNum & (0xffffffff << (32 - prefix))) + (i * blockSize);
            int firstHost = networkAddress + 1;
            int lastHost = networkAddress + blockSize - 2;
            int broadcastAddress = networkAddress + blockSize - 1;

            System.out.println("\nSubnet " + (i + 1) + ":");
            System.out.println("  Network Address: " + intToIP(networkAddress) +
                    " (" + intToBinaryIP(networkAddress) + ")");
            System.out.println("  First Usable Host: " + intToIP(firstHost) +
                    " (" + intToBinaryIP(firstHost) + ")");
            System.out.println("  Last Usable Host: " + intToIP(lastHost) +
                    " (" + intToBinaryIP(lastHost) + ")");
            System.out.println("  Broadcast Address: " + intToIP(broadcastAddress) +
                    " (" + intToBinaryIP(broadcastAddress) + ")");
        }
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);

        System.out.print("Enter IP Address (e.g. 192.168.1.10): ");
        String ip = sc.nextLine();

        System.out.print("Enter CIDR Prefix (e.g. 24): ");
        int prefix = sc.nextInt();

        System.out.print("Enter number of subnets required: ");
        int requiredSubnets = sc.nextInt();

        calculateSubnetDetails(ip, prefix, requiredSubnets);

        sc.close();
    }
}
