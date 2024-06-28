class ClasesYObjetos{
	public static void main(String [] arg){
		Persona persona;
		Persona persona2 = new Persona();
		persona= new Persona(1111111,"Juan Perez",3456.78);
		persona.imprimirDatos();
		persona2.SetDni(222222);
		persona2.SetNombre("Maria Quispe");
		persona2.SetSueldo(64547.63);
		persona2.imprimirDatos();
		
		Persona persona3= new Persona(persona2);
		persona3.imprimirDatos();
		persona3.SetDni(55667788);
		persona3.imprimirDatos();
		persona2.imprimirDatos();
	}
}