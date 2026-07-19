export interface Collector<TSnapshot> {
  poll(): Promise<TSnapshot[]>;
}
