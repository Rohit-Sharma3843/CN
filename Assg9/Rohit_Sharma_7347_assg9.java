import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Scanner;

public class one {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        while (true) {
            System.out.println("\n--- IP ↔ Domain Converter ---");
            System.out.println("1. Domain to IPs");
            System.out.println("2. IP to Domain");
            System.out.println("3. Exit");
            System.out.print("Choose an option (1, 2, or 3): ");
            
            int choice;
            try {
                choice = Integer.parseInt(scanner.nextLine().trim());
            } catch (NumberFormatException e) {
                System.out.println("Please enter a valid number.");
                continue;
            }

            if (choice == 3) {
                System.out.println("Exiting program. Goodbye!");
                break;
            }

            try {
                if (choice == 1) {
                    System.out.print("Enter domain name: ");
                    String domain = scanner.nextLine().trim();
                    InetAddress[] addresses = InetAddress.getAllByName(domain);
                    
                    System.out.println("IP addresses for " + domain + ":");
                    for (InetAddress address : addresses) {
                        System.out.println(" - " + address.getHostAddress());
                    }

                } else if (choice == 2) {
                    System.out.print("Enter IP address: ");
                    String ip = scanner.nextLine().trim();
                    InetAddress inetAddress = InetAddress.getByName(ip);
                    System.out.println("Domain name: " + inetAddress.getHostName());
                } else {
                    System.out.println("Invalid option selected.");
                }
            } catch (UnknownHostException e) {
                System.out.println("Error: " + e.getMessage());
            }
        }
        scanner.close();
    }
}
