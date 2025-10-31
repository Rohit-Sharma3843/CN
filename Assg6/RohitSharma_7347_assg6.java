import java.util.*;

public class RohitSharma_7347_assg6 {
    static final int INF = 9999;

    public static void dijkstra(int[][] graph, int src, String[] nodes) {
        int V = graph.length;
        int[] dist = new int[V];
        boolean[] visited = new boolean[V];
        int[] parent = new int[V];

        Arrays.fill(dist, INF);
        Arrays.fill(parent, -1);
        dist[src] = 0;

        for (int count = 0; count < V - 1; count++) {
            int u = minDistance(dist, visited);
            visited[u] = true;

            for (int v = 0; v < V; v++) {
                if (!visited[v] && graph[u][v] != INF &&
                        dist[u] + graph[u][v] < dist[v]) {
                    dist[v] = dist[u] + graph[u][v];
                    parent[v] = u;
                }
            }
        }

        printTable(src, dist, parent, nodes);
    }

    public static int minDistance(int[] dist, boolean[] visited) {
        int min = INF, minIndex = -1;
        for (int v = 0; v < dist.length; v++) {
            if (!visited[v] && dist[v] <= min) {
                min = dist[v];
                minIndex = v;
            }
        }
        return minIndex;
    }

    public static void printTable(int src, int[] dist, int[] parent, String[] nodes) {
        System.out.println("\nSource: " + nodes[src]);
        System.out.printf("%-12s%-8s%-20s\n", "Destination", "Cost", "Intermediate Points");
        for (int i = 0; i < dist.length; i++) {
            if (i == src) {
                System.out.printf("%-12s%-8d%-20s\n", nodes[i], 0, "---");
            } else {
                String path = getPath(i, parent, nodes);
                System.out.printf("%-12s%-8d%-20s\n", nodes[i], dist[i], path);
            }
        }
    }

    public static String getPath(int v, int[] parent, String[] nodes) {
        List<String> path = new ArrayList<>();
        while (parent[v] != -1) {
            path.add(nodes[v]);
            v = parent[v];
        }
        Collections.reverse(path);
        return path.isEmpty() ? "---" : String.join(" -> ", path);
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);

        System.out.print("Enter number of nodes: ");
        int n = sc.nextInt();
        sc.nextLine();

        String[] nodes = new String[n];
        System.out.println("Enter node names:");
        for (int i = 0; i < n; i++) {
            nodes[i] = sc.nextLine().trim();
        }

        int[][] graph = new int[n][n];
        System.out.println("Enter adjacency matrix values (use " + INF + " for no edge):");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                System.out.print("Enter value [" + i + "][" + j + "] of adjacency matrix: ");
                graph[i][j] = sc.nextInt();
            }
        }

        for (int i = 0; i < n; i++) {
            dijkstra(graph, i, nodes);
        }
    }
}
