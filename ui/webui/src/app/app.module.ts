import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { MainComponent } from './pages/main/main.component';
import { LoginComponent } from './pages/login/login.component';
import { SystemComponent } from './pages/system/system.component';
import { ConfigComponent } from './pages/config/config.component';

@NgModule({
  declarations: [
    AppComponent,
    MainComponent,
    LoginComponent,
    SystemComponent,
    ConfigComponent
  ],
  imports: [
    BrowserModule,
    AppRoutingModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
